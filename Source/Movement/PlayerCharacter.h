// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MOVEMENT_API APlayerCharacter : public APawn
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

public:
	// Sets default values for this pawn's properties
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	float movementNumber;

	float lastMovementNumber;

	float movementMultiplier;

	float startingWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool mirror;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool spawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool dark;

	float height;

protected:
	// Called when the game starts or when spawned
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	//void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void SelectDeployable(float Value);

	void RunDark(float Value);

	void ForwardMovement();

	void BackwardMovement();

	void FullForward();

	void FullBackward();

	void StopMovement();

	void MoveCharacter(float movementNumber, float lastMovementNumber, float movementMultiplier);

	void SpawnMirror(FVector start, FVector end);

	void SpawnDecoy(FVector start, FVector end);

	void SpawnProbe(FVector start, FVector end);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	virtual void BeginPlay() override;

public:	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Tick(float DeltaTime) override;

	void Place();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<AActor*> Deployables;

	int currentDeployable;

	// Projectile class to spawn.
	UPROPERTY(EditDefaultsOnly, Category = Deployable)
		TSubclassOf<class AMirrorShot> MirrorShotClass;

	UPROPERTY(EditDefaultsOnly, Category = Deployable)
		TSubclassOf<class ADecoy> DecoyClass;

	UPROPERTY(EditDefaultsOnly, Category = Deployable)
		TSubclassOf<class AProbe> ProbeClass;

	UPROPERTY(EditDefaultsOnly, Category = Objective)
		TSubclassOf<class AObjective> ObjectiveClass;

	
	
};
