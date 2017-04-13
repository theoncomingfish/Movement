// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Movement.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "EngineUtils.h"
#include "MovementCharacter.h"
#include "MirrorField.h"
#include "Decoy.h"
#include "Probe.h"
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
	startingWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	dark = false;

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
	GetCharacterMovement()->bOrientRotationToMovement = false;
	MoveCharacter(movementNumber, lastMovementNumber, movementMultiplier);
	
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
	if ( (Controller != NULL) && (Value != 0.0f) && !dark)
	{
		// find out which way is right
		FRotator Rotation = GetActorRotation();
		Rotation = Rotation.Add(0.0f, 2*Value, 0.0f);
		
		//rotate actor 
		SetActorRotation(Rotation);
	}
}

void AMovementCharacter::SelectDeployable(float Value)
{
	if (Value >= 1.0f) {
		currentDeployable = static_cast<int>(Value);
	}
}

void AMovementCharacter::ForwardMovement() {
	if (movementNumber < 4 && !dark) {
		movementNumber++;
	}
}

void AMovementCharacter::BackwardMovement() {
	if (movementNumber > -4 && !dark) {
		movementNumber--;
	}
}

void AMovementCharacter::StopMovement() {
	if (!dark) {
		movementNumber = 0.0f;
	}
}

void AMovementCharacter::FullForward() {
	if (!dark) {
		movementNumber = 4.0f;
	}
}

void AMovementCharacter::FullBackward() {
	if (!dark) {
		movementNumber = -4.0f;
	}
}

void AMovementCharacter::MoveCharacter(float aMovementNumber, float aLastMovementNumber, float aMovementMultiplier) {
	if (aMovementNumber > 0 && aLastMovementNumber >= 0) {
		if (aMovementMultiplier != aMovementNumber && aMovementNumber > aMovementMultiplier) {
			movementMultiplier += (0.01f * aMovementNumber);
		}
		else if (aMovementNumber != aMovementMultiplier && aMovementNumber < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, 1.0f);
		lastMovementNumber = movementNumber;
	}
	else if (aMovementNumber > 0 && aLastMovementNumber < 0) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier -= (0.01f * aMovementNumber);
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, -1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	else if (aMovementNumber < 0.0f && aLastMovementNumber <= 0.0f) {
		if (aMovementMultiplier != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) > aMovementMultiplier) {
			movementMultiplier -= (0.01f * aMovementNumber);
		}
		else if (aMovementNumber != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, -1.0f);
		lastMovementNumber = movementNumber;
	}
	else if (aMovementNumber < 0.0f && aLastMovementNumber > 0.0f) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier += (0.01f * aMovementNumber);
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, 1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	else if (aMovementNumber == 0.0f) {
		if (aMovementMultiplier > aMovementNumber) {
			movementMultiplier -= 0.01f;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			if (aLastMovementNumber > 0) {
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, 1.0f);
			}
			else if (aLastMovementNumber < 0) {
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
	if (!dark) {
		FHitResult* Result = new FHitResult();
		FVector Start = GetActorLocation();
		FVector Forward = Controller->GetActorForwardVector();
		FVector End = (Forward * 5000.f) + Start;
		FCollisionQueryParams* TraceParams = new FCollisionQueryParams();
		FColor color;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::FromInt(currentDeployable));
		switch (currentDeployable) {
		case 1:	color = FColor::Red;
			break;
		case 2: color = FColor::Blue;
			break;
		case 3: color = FColor::Purple;
			break;
		default: color = FColor::Black;
			break;
		}




		if (GetWorld()->LineTraceSingleByChannel(*Result, Start, End, ECC_Visibility, *TraceParams)) {
			End = Result->ImpactPoint;
			DrawDebugLine(GetWorld(), Start, End, color, 1.0, 12, 200.0f);
			if (currentDeployable == 1) {
				SpawnMirror(End);
			}
			else if (currentDeployable == 2) {
				SpawnDecoy(Start, End);
			}
			else if (currentDeployable == 3) {
				SpawnProbe(Start, End);
			}
		}
		else {
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, 1.0, 12, 200.0f);
			if (currentDeployable == 1) {
				SpawnMirror(End);
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

void AMovementCharacter::SpawnMirror(FVector End) {
	if (MirrorFieldClass) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.
			AMirrorField* Mirror = World->SpawnActor<AMirrorField>(MirrorFieldClass, End, GetControlRotation(), SpawnParams);


		}
	}
}

void AMovementCharacter::SpawnDecoy(FVector Start, FVector End) {
	if (DecoyClass) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.
			FRotator Rotation = Controller->GetControlRotation();
			FRotator YawRotation(0, Rotation.Yaw, 0);

			

			ADecoy* decoy = World->SpawnActor<ADecoy>(DecoyClass, Start, GetControlRotation(), SpawnParams);
			decoy->end = End;

		}
	}
}

void AMovementCharacter::SpawnProbe(FVector Start, FVector End) {
	if (ProbeClass) {
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			// Spawn the projectile at the muzzle.

			AProbe* probe = World->SpawnActor<AProbe>(ProbeClass, Start, GetControlRotation(), SpawnParams);
			probe->end = End;

		}
	}
}