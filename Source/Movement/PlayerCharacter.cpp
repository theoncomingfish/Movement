// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "EngineUtils.h"
#include "MirrorField.h"
#include "MirrorShot.h"
#include "Decoy.h"
#include "Probe.h"
#include "Objective.h"
#include <string>
#include "PlayerCharacter.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

