// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyTank.h"
#include "TankAiming.h"
#include "TankPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TANK_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick(float detaltime);

	AMyTank* GetControlledTank();
	
	void AmiToTarget();

	bool GetSightRayHitLocation(FVector& hitlocation);

	bool GetLookVectorHitLocation(const FVector& lookdirection, FVector& outhitlocation);

	float mCrossHairsX = 0.5f;
	float mCrossHairsY = 0.3f;

	UPROPERTY(EditAnywhere)
	float mLineLength = 1000000.0f;
};
