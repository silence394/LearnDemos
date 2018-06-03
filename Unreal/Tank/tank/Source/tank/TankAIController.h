// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MyTank.h"
#include "Engine/World.h"
#include "TankAIController.generated.h"

/**
 * 
 */
UCLASS()
class TANK_API ATankAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMyTank* GetControlledTank();
	AMyTank* GetPlayerTank();
	
	virtual void BeginPlay();
	
};
