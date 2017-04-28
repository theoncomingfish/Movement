// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "MovementCharacter.generated.h"

UCLASS(config=Game)
class AMovementCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AMovementCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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

