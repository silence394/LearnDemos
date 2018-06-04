// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TankBarrel.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TANK_API UTankBarrel : public UStaticMeshComponent
{
	GENERATED_BODY()
	
	
	UPROPERTY(EditAnywhere, Category = "Setup")
	float mMaxDegreePerSecond = 20.0f;
	UPROPERTY(EditAnywhere, Category = "Setup")
	float mMaxDegree = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Setup")
	float mMinDegree = -40.0f;
};
