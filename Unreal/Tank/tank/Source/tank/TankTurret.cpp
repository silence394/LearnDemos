// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTurret.h"

void UTankTurret::MoveTurret(const FVector& dir)
{
	FRotator currot = GetForwardVector().Rotation();
	FRotator aimrot = dir.Rotation();

	float changeyaw = aimrot.Yaw - currot.Yaw;
	if (changeyaw > 180.0f)
		changeyaw -= 360.0f;
	else if (changeyaw < -180.0f)
		changeyaw += 360.0f;

	float relativespeed = FMath::Clamp<float>(changeyaw, -1.0f, 1.0f);
	float rotchange = relativespeed * mMaxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	float newrot = rotchange + currot.Yaw;
	SetRelativeRotation(FRotator(0.0f, newrot, 0.0f));
}