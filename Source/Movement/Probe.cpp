// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement.h"
#include "Probe.h"


// Sets default values
AProbe::AProbe()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProbe::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AProbe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

