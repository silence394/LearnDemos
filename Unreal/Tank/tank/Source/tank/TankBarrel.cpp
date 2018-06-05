// Fill out your copyright notice in the Description page of Project Settings.

#include "TankBarrel.h"

void UTankBarrel::ChangePitch(const FVector& dir)
{
		FRotator currot = GetForwardVector().Rotation();
	FRotator aimrot = dir.Rotation();

	float changepitch = aimrot.Pitch - currot.Pitch;
	float relativespeed = FMath::Clamp<float>(changeyaw, -1.0f, 1.0f);
	float change = relativespeed * mMaxDegreePerSecond * GetWorld()->DeltaTimeSeconds;

	float newrot = change + currot;
	newrot = FMath::Clamp<float>(newrot, mMinDegree, mMaxDegree);

	SetRelativeRotation(FRotator(newrot, 0.0f, 0.0f));
}


