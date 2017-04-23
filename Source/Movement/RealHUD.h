// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "RealHUD.generated.h"

/**
 * 
 */
UCLASS()
class MOVEMENT_API ARealHUD : public AHUD
{
	GENERATED_BODY()
	
	
protected:
	// This will be drawn at the center of the screen.
	UPROPERTY(EditDefaultsOnly)
		UTexture2D* CrosshairTexture;

public:
	// Primary draw call for the HUD.
	virtual void DrawHUD() override;
	
};
