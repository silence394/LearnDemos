// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUpBase.h"
#include "MyTank.h"
#include "MyPickUpHP.generated.h"

/**
 * 
 */
UCLASS()
class TANK_API AMyPickUpHP : public APickUpBase
{
	GENERATED_BODY()

public:
	virtual void UseItem(AActor* actor);

	UPROPERTY(EditAnywhere, Category = "Pickup")
	int mHPAdded = 20;
};
