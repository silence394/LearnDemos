// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TankTrack.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TANK_API UTankTrack : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay();

	UFUNCTION()
	void OnHit( UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetThrottle(float throttle);

	void DriveTrack();

	UPROPERTY(EditAnywhere, Category = "Setup")
	float mTankMaxDriveForce = 40000000.0f;

	float mThrottle = 0.0f;
};
