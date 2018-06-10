// Fill out your copyright notice in the Description page of Project Settings.

#include "MyTank.h"

// Sets default values
AMyTank::AMyTank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyTank::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyTank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyTank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float AMyTank::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damage = FMath::Clamp<int>(FPlatformMath::RoundToInt(Damage), 0, mCurrentHP);
	mCurrentHP -= damage;
	if (mCurrentHP <= 0)
		UE_LOG(LogTemp, Warning, TEXT("Tank = %s die"), *GetName());

	return damage;
}