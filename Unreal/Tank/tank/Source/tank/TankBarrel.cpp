// Fill out your copyright notice in the Description page of Project Settings.

#include "TankBarrel.h"

void UTankBarrel::MoveBarrel(const FVector& dir)
{
	FRotator currot = GetForwardVector().Rotation();
	FRotator aimrot = dir.Rotation();

	float mDetalPitch = aimrot.Pitch - currot.Pitch;
	float relativespeed = FMath::Clamp<float>(mDetalPitch, -1.0f, 1.0f);
	float change = relativespeed * mMaxDegreePerSecond * GetWorld()->DeltaTimeSeconds;

	float newrot = change + RelativeRotation.Pitch;
	newrot = FMath::Clamp<float>(newrot, mMinDegree, mMaxDegree);

	SetRelativeRotation(FRotator(newrot, 0.0f, 0.0f));
}

float UTankBarrel::GetDetalPitch() const
{
	return mDetalPitch;
}