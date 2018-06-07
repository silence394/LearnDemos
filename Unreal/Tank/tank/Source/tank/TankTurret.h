// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TankTurret.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TANK_API UTankTurret : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	void	MoveTurret(const FVector& dir);
	float	GetDetalYaw() const;

	UPROPERTY(EditAnywhere, Category = "Setup")
	float	mMaxDegreePerSecond = 20.0f;

	float	mDetalYaw = 0.0f;
};
