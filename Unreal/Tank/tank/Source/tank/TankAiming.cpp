// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAiming.h"


// Sets default values for this component's properties
UTankAiming::UTankAiming()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTankAiming::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTankAiming::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTankAiming::Init(UTankBarrel* barrel, UTankTurret* turret)
{
	mBarrel = barrel;
	mTurret = turret;
}

void UTankAiming::AmiAt(const FVector& hit)
{
	if (mBarrel == nullptr || mTurret == nullptr)
		return;

	FVector firedir;
	FVector start = mBarrel->GetSocketLocation("FireLocation");

	bool ret = false;
	ret = UGameplayStatics::SuggestProjectileVelocity(this, firedir, start, hit, mLauchSpeed, false, 0.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace);

	if (ret)
	{
		// Turn around.
		UE_LOG(LogTemp, Warning, TEXT("fire!!, dir = %s"), *firedir.GetSafeNormal().ToString());
	}
}