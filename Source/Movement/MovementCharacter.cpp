// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Movement.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "EngineUtils.h"
#include "MovementCharacter.h"
#include "MirrorField.h"
#include "MirrorShot.h"
#include "Decoy.h"
#include "Probe.h"
#include "Objective.h"
#include <string>

//////////////////////////////////////////////////////////////////////////
// AMovementCharacter

AMovementCharacter::AMovementCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	//set initial movement input
	movementNumber = 0.0f;
	lastMovementNumber = 0.0f;
	movementMultiplier = 0.0f;
	movementSliderValue = movementMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	startingWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	//initialize variables
	dark = false;
	mirror = false;
	mirrorOut = false;
	decoyOut = false;
	probeOut = false;
	leftTurn = false;
	rightTurn = false;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	Deployables = TArray<AActor*>();
	currentDeployable = 1;
	
}

void AMovementCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//trigger movement on tick
	GetCharacterMovement()->bOrientRotationToMovement = false;
	MoveCharacter(movementNumber, lastMovementNumber, movementMultiplier);
	rightTurn = false;
	leftTurn = false;
	
}

void AMovementCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ObjectiveClass) {
		UWorld* World = GetWorld();
		if (World)
		{
			FVector Start = GetActorLocation();
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.

			AObjective* objective = World->SpawnActor<AObjective>(ObjectiveClass, Start, GetControlRotation(), SpawnParams);
			

		}
	}

	height = GetActorLocation().Z;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMovementCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ForwardMovement", IE_Pressed, this, &AMovementCharacter::ForwardMovement);
	PlayerInputComponent->BindAction("ForwardMovement", IE_DoubleClick, this, &AMovementCharacter::FullForward);
	PlayerInputComponent->BindAction("BackwardMovement", IE_Pressed, this, &AMovementCharacter::BackwardMovement);
	PlayerInputComponent->BindAction("BackwardMovement", IE_DoubleClick, this, &AMovementCharacter::FullBackward);
	PlayerInputComponent->BindAction("StopMovement", IE_Pressed, this, &AMovementCharacter::StopMovement);

	//PlayerInputComponent->BindAxis("MoveForward", this, &AMovementCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMovementCharacter::MoveRight);

	PlayerInputComponent->BindAxis("RunDark", this, &AMovementCharacter::RunDark);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMovementCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMovementCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMovementCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMovementCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMovementCharacter::OnResetVR);

	//Handle deployables
	PlayerInputComponent->BindAction("Place", IE_Pressed, this, &AMovementCharacter::Place);
	PlayerInputComponent->BindAxis("SelectDeployable", this, &AMovementCharacter::SelectDeployable);

	//mouse wheel scroll functionality
	PlayerInputComponent->BindAction("IncrementDeployable", IE_Pressed, this, &AMovementCharacter::IncrementDeployable);
	PlayerInputComponent->BindAction("DecrementDeployable", IE_Pressed, this, &AMovementCharacter::DecrementDeployable);
	
}


void AMovementCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMovementCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMovementCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMovementCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMovementCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMovementCharacter::RunDark(float Value) {
	//handle running dark
	if (Value == 1.0f) {
		dark = true;
	}
	else {
		dark = false;
	}
}
/*
void AMovementCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		//const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, Value);
	}
}
*/
void AMovementCharacter::MoveRight(float Value)
{
	//function for handling player rotation
	if ( (Controller != NULL) && (Value != 0.0f) && !dark)
	{
		// find out which way is right
		FRotator Rotation = GetActorRotation();
		Rotation = Rotation.Add(0.0f, 2*Value, 0.0f);
		leftTurn = false;
		rightTurn = false;
		if (Value < 0) {
			leftTurn = true;
			
		}
		else if (Value > 0) {
			
			rightTurn = true;
		}
		
		//rotate actor 
		SetActorRotation(Rotation);
	}
}

void AMovementCharacter::SelectDeployable(float Value)
{
	//deployable select via number keys
	if (Value >= 1.0f) {
		currentDeployable = static_cast<int>(Value);
	}
}

void AMovementCharacter::IncrementDeployable() {
	//deployable select via mouse wheel
	if (currentDeployable < 3) {
		currentDeployable++;
	}
}

void AMovementCharacter::DecrementDeployable() {
	//deployable select via mouse wheel
	if (currentDeployable > 1) {
		currentDeployable--;
	}
}

void AMovementCharacter::ForwardMovement() {
	//increment the movement number
	if (movementNumber < 4 && !dark) {
		movementNumber++;
	}
}

void AMovementCharacter::BackwardMovement() {
	//decrement the movement number
	if (movementNumber > -4 && !dark) {
		movementNumber--;
	}
}

void AMovementCharacter::StopMovement() {
	//set movement number to 0
	if (!dark) {
		movementNumber = 0.0f;
	}
}

void AMovementCharacter::FullForward() {
	//set movement value to highest value
	if (!dark) {
		movementNumber = 4.0f;
	}
}

void AMovementCharacter::FullBackward() {
	//set movement value to lowest value
	if (!dark) {
		movementNumber = -4.0f;
	}
}

void AMovementCharacter::MoveCharacter(float aMovementNumber, float aLastMovementNumber, float aMovementMultiplier) {
	//handle forward movement changes
	if (aMovementNumber > 0 && aLastMovementNumber >= 0) {
		if (aMovementMultiplier != aMovementNumber && aMovementNumber > aMovementMultiplier) {
			movementMultiplier += (0.01f * aMovementNumber);
			movementSliderValue = movementMultiplier;
		}
		else if (aMovementNumber != aMovementMultiplier && aMovementNumber < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
			movementSliderValue = movementMultiplier;
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, 1.0f);
		lastMovementNumber = movementNumber;
	}
	
	else if (aMovementNumber > 0 && aLastMovementNumber < 0) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier -= (0.01f * aMovementNumber);
			movementSliderValue = (-1 * movementMultiplier);
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, -1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	//handle backward movement changes
	else if (aMovementNumber < 0.0f && aLastMovementNumber <= 0.0f) {
		if (aMovementMultiplier != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) > aMovementMultiplier) {
			movementMultiplier -= (0.01f * aMovementNumber);
			movementSliderValue = (-1 * movementMultiplier);
		}
		else if (aMovementNumber != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
			movementSliderValue = (-1 * movementMultiplier);
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, -1.0f);
		lastMovementNumber = movementNumber;
	}
	else if (aMovementNumber < 0.0f && aLastMovementNumber > 0.0f) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier += (0.01f * aMovementNumber);
			movementSliderValue = movementMultiplier;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, 1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	//handle stopping
	else if (aMovementNumber == 0.0f) {
		if (aMovementMultiplier > aMovementNumber) {
			movementMultiplier -= 0.01f;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			if (aLastMovementNumber > 0) {
				movementSliderValue = movementMultiplier;
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, 1.0f);
			}
			else if (aLastMovementNumber < 0) {
				movementSliderValue = (-1 * movementMultiplier);
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, -1.0f);
			}
		}
		else {
			lastMovementNumber = 0.0f;
		}
	}
}

void AMovementCharacter::Place()
{
	//handle firing of deployables
	if (!dark) {
		FHitResult* Result = new FHitResult();
		FVector Start = GetActorLocation();
		FVector Forward = Controller->GetActorForwardVector();
		FVector End = (Forward * 5000.f) + Start;
		End.Z = height;
		FCollisionQueryParams* TraceParams = new FCollisionQueryParams();
		FColor color;


		if (GetWorld()->LineTraceSingleByChannel(*Result, Start, End, ECC_Visibility, *TraceParams)) {
			End = Result->ImpactPoint;
			End.Z = height;
			
			if (currentDeployable == 1) {
				SpawnMirror(Start, End);
			}
			else if (currentDeployable == 2) {
				SpawnDecoy(Start, End);
			}
			else if (currentDeployable == 3) {
				SpawnProbe(Start, End);
			}
		}
		else {
			End.Z = height;
			
			if (currentDeployable == 1) {
				SpawnMirror(Start, End);
			}
			else if (currentDeployable == 2) {
				SpawnDecoy(Start, End);
			}
			else if (currentDeployable == 3) {
				SpawnProbe(Start, End);
			}
		}
	}
}

void AMovementCharacter::SpawnMirror(FVector Start, FVector End) {
	//spawn mirror field deployable
	if (MirrorShotClass && !mirrorOut) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.
			FRotator rotation = GetControlRotation();
			rotation.Pitch = GetActorRotation().Pitch;
			AMirrorShot* Mirror = World->SpawnActor<AMirrorShot>(MirrorShotClass, Start, rotation, SpawnParams);
			Mirror->end = End;
			mirror = true;
			mirrorOut = true;
		}
	}
}

void AMovementCharacter::SpawnDecoy(FVector Start, FVector End) {
	//spawn decoy deployable
	if (DecoyClass && !decoyOut) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.
			FRotator Rotation = GetControlRotation();
			Rotation.Pitch = GetActorRotation().Pitch;

			

			ADecoy* decoy = World->SpawnActor<ADecoy>(DecoyClass, Start, Rotation, SpawnParams);
			decoy->end = End;
			decoyOut = true;
		}
	}
}

void AMovementCharacter::SpawnProbe(FVector Start, FVector End) {
	//spawn probe deployable
	if (ProbeClass && !probeOut) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.
			FRotator Rotation = GetControlRotation();
			Rotation.Pitch = GetActorRotation().Pitch;

			AProbe* probe = World->SpawnActor<AProbe>(ProbeClass, Start, Rotation, SpawnParams);
			probe->end = End;
			probeOut = true;
		}
	}
}

