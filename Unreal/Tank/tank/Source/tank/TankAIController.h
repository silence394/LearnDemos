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
	virtual void SetPawn(APawn* inpawn);

	virtual void BeginPlay();
	virtual void Tick(float detaltime);

	AMyTank*	GetControlledTank();
	AMyTank*	GetPlayerTank();

	UFUNCTION()
	void OnControlTankDeath();

	UPROPERTY(EditAnywhere, Category = "Setup")
	float mAcceptanceRadius = 3000.0f;
};