// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyTank.h"
#include "TankAiming.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "TankPlayerController.generated.h"

UCLASS()
class TANK_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void SetPawn(APawn* inpawn);
	virtual void BeginPlay() override;
	virtual void Tick(float detaltime);

	UFUNCTION(BlueprintCallable)
	AMyTank* GetControlledTank();

	UFUNCTION()
	void OnControlTankDeath();

	void AmiToTarget();
	bool GetSightRayHitLocation(FVector& hitlocation);
	bool GetLookVectorHitLocation(const FVector& lookdirection, FVector& outhitlocation);

	UFUNCTION()
	void Reborn();

	float mCrossHairsX = 0.5f;
	float mCrossHairsY = 0.3f;

	UPROPERTY(EditAnywhere)
	float mLineLength = 1000000.0f;

	FTimerHandle	mTankRebornTimer;

	UPROPERTY(EditAnywhere)
	float			mRebornTime = 5.0f;

	AMyTank*	mMyTank = nullptr;
};
