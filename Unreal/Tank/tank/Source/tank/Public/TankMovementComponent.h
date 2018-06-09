// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/NavMovementComponent.h"
#include "TankTrack.h"
#include "TankMovementComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TANK_API UTankMovementComponent : public UNavMovementComponent
{
	GENERATED_BODY()

public:
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed);

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void Init(UTankTrack* left, UTankTrack* right);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void MoveForward(float throttle);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void MoveRight(float throttle);

private:
	UTankTrack * mLeftTrack = nullptr;
	UTankTrack * mRightTrack = nullptr;
};
