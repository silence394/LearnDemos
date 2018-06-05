// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTurret.h"

void UTankTurret::RotateTurret()
{

}

void UTankTurret::MoveTurret(const FVector& dir)
{
	FRotator currot = GetForwardVector().Rotation();
	FRotator aimrot = dir.Rotation();

	float changeyaw = aimrot.Yaw - currot.Yaw;
	float relativespeed = FMath::Clamp<float>(changeyaw, -1.0f, 1.0f);
	float change = relativespeed * mMaxDegreePerSecond * GetWorld()->DeltaTimeSeconds;

	if (change > 180.0f)
		change -= 360.0f;
	else if (change < -180.0f)
		change += 360.0f

	float newrot = change + currot;
//	newrot = FMath::Clamp<float>(newrot, mMinDegree, mMaxDegree);

	SetRelativeRotation(FRotator(0.0f, newrot, 0.0f));
}
