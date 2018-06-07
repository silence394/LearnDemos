// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TankBarrel.h"
#include "TankTurret.h"
#include "Kismet/GameplayStatics.h"
#include "Public/Projectile.h"
#include "TankAiming.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TANK_API UTankAiming : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTankAiming();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void Init(UTankBarrel* barrel, UTankTurret* turret);

	UFUNCTION(BlueprintCallable, Category = "GamePlay")
	void Fire();

	void AmiAt(const FVector& hit);

	UTankBarrel*	mBarrel = nullptr;
	UTankTurret*	mTurret = nullptr;

	UPROPERTY(EditAnywhere)
	float mLauchSpeed = 10000.0f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> mProjectileType;
};
