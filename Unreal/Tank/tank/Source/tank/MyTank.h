// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyTank.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTankDelegate);

UCLASS()
class TANK_API AMyTank : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyTank();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	// 不需要节点连接顺序
	UFUNCTION(BlueprintPure, Category = "Tank")
	float GetHealthPercentage();

	void SetHP(int health);
	int GetHPMax();


	FTankDelegate	mOnDeath;

private:
	UPROPERTY(EditAnywhere, Category = "HP")
	int32	mMaxHP = 100;

	UPROPERTY(VisibleAnywhere, Category = "HP")
	int32	mCurrentHP = 100;
};
