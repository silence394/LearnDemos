// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTurret.h"

float UTankTurret::GetDetalYaw() const
{
	return mDetalYaw;
}

void UTankTurret::MoveTurret(const FVector& dir)
{
	FRotator currot = GetForwardVector().Rotation();
	FRotator aimrot = dir.Rotation();

	mDetalYaw = aimrot.Yaw - currot.Yaw;
	if (mDetalYaw > 180.0f)
		mDetalYaw -= 360.0f;
	else if (mDetalYaw < -180.0f)
		mDetalYaw += 360.0f;

	float relativespeed = FMath::Clamp<float>(mDetalYaw, -1.0f, 1.0f);
	float rotchange = relativespeed * mMaxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	float newrot = rotchange + RelativeRotation.Yaw;
	SetRelativeRotation(FRotator(0.0f, newrot, 0.0f));
}