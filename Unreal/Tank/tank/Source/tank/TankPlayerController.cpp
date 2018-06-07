// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AMyTank* tank = GetControlledTank();

	if (tank == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error: control Tank is null"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Control Tank is %s"), *tank->GetName());
	}
}

void ATankPlayerController::Tick(float detaltime)
{
	AmiToTarget();
}

AMyTank* ATankPlayerController::GetControlledTank()
{
	return Cast<AMyTank>(GetPawn());
}

void ATankPlayerController::AmiToTarget()
{
	FVector hit;
	if (GetSightRayHitLocation(hit))
	{
		GetControlledTank()->FindComponentByClass<UTankAiming>()->AmiAt(hit);
		UE_LOG(LogTemp, Warning, TEXT("hit location : %s"), *hit.ToString());
	}
}

bool ATankPlayerController::GetSightRayHitLocation(FVector& hitlocation)
{
	int32 vpsizex, vpsizey;
	GetViewportSize(vpsizex, vpsizey);
	FVector2D screenlocation = FVector2D(mCrossHairsX * vpsizex, mCrossHairsY * vpsizey);

	FVector worldlocation;
	FVector worlddirection;

	if (DeprojectScreenPositionToWorld(screenlocation.X, screenlocation.Y, worldlocation, worlddirection))
		return GetLookVectorHitLocation(worlddirection, hitlocation);
	else
		return false;
}

bool ATankPlayerController::GetLookVectorHitLocation(const FVector& lookdirection, FVector& outhitlocation)
{
	FVector start = PlayerCameraManager->GetCameraLocation();
	FVector end = start + lookdirection * mLineLength;

	FHitResult result;
	if (GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_Visibility) && result.Actor != GetControlledTank())
	{
		outhitlocation = result.Location;
		return true;
	}
	else
	{
		outhitlocation = FVector(0.0f);
		return false;
	}
}
