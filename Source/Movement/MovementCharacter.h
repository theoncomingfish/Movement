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

	float movementNumber;

	float lastMovementNumber;

	float movementMultiplier;

	float startingWalkSpeed;

	bool dark;

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

	void SpawnMirror(FVector end);

	void SpawnDecoy(FVector start, FVector end);

	void SpawnProbe(FVector start, FVector end);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

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
		TSubclassOf<class AMirrorField> MirrorFieldClass;

	UPROPERTY(EditDefaultsOnly, Category = Deployable)
		TSubclassOf<class ADecoy> DecoyClass;

	UPROPERTY(EditDefaultsOnly, Category = Deployable)
		TSubclassOf<class AProbe> ProbeClass;
};

