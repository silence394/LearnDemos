// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"

void UTankTrack::SetThrottle(float throttle)
{
	FVector force = mTankMaxDriveForce * throttle * GetForwardVector();
	FVector forcelocation = GetComponentLocation();

	UPrimitiveComponent* rootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	rootComponent->AddForceAtLocation(force, forcelocation);
}