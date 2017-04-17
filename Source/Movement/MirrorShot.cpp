// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement.h"
#include "MirrorShot.h"


// Sets default values
AMirrorShot::AMirrorShot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMirrorShot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMirrorShot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

