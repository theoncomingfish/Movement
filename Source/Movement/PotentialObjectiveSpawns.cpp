// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement.h"
#include "PotentialObjectiveSpawns.h"


// Sets default values
APotentialObjectiveSpawns::APotentialObjectiveSpawns()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APotentialObjectiveSpawns::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APotentialObjectiveSpawns::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

