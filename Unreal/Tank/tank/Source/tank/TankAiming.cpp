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

	if (mBarrel == nullptr || mTurret == nullptr)
		return;

	if ((FPlatformTime::Seconds() - mLastFireTime) < mReloadTime)
	{
		mFireState = EFireState::FS_Reloading;
		return;
	}

	if (FMath::Abs(mTurret->GetDetalYaw()) > 3.0f || FMath::Abs(mBarrel->GetDetalPitch()) > 3.0f)
		mFireState = EFireState::FS_Aiming;
	else
		mFireState = EFireState::FS_Locked;
}

void UTankAiming::Init(UTankBarrel* barrel, UTankTurret* turret)
{
	mBarrel = barrel;
	mTurret = turret;
}

void UTankAiming::Fire()
{
	if (mBarrel == nullptr || mProjectileType == nullptr)
		return;

	bool isreloaded = (FPlatformTime::Seconds() - mLastFireTime) > mReloadTime;
	if (isreloaded == false)
		return;

	AProjectile* pojectile = GetWorld()->SpawnActor<AProjectile>(mProjectileType, mBarrel->GetSocketLocation(FName("FireLocation")), mBarrel->GetSocketRotation(FName("FireLocation")));
	pojectile->LaunchProjectile(mLauchSpeed);

	mLastFireTime = FPlatformTime::Seconds();
}

void UTankAiming::AmiAt(const FVector& hit)
{
	if (mBarrel == nullptr || mTurret == nullptr)
		return;

	FVector firedir;
	FVector start = mBarrel->GetSocketLocation(FName("FireLocation"));

	bool ret = false;
	ret = UGameplayStatics::SuggestProjectileVelocity(this, firedir, start, hit, mLauchSpeed, false, 0.0f, 0.0f, ESuggestProjVelocityTraceOption::DoNotTrace);

	if (ret)
	{
		// Turn around.
		mTurret->MoveTurret(firedir);
		mBarrel->MoveBarrel(firedir);
	}
}