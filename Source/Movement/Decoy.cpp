// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement.h"
#include "Decoy.h"


// Sets default values
ADecoy::ADecoy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADecoy::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADecoy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

