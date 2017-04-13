// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Decoy.generated.h"

UCLASS()
class MOVEMENT_API ADecoy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADecoy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector end;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* mat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* mat2;
};
