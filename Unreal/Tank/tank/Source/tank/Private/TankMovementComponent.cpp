// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"

void UTankMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	UE_LOG(LogTemp, Warning, TEXT("Vel = %s"), *MoveVelocity.ToString())


	FVector vel = MoveVelocity.GetSafeNormal();
	FVector forward = GetOwner()->GetActorForwardVector().GetSafeNormal();
	FVector right = GetOwner()->GetActorRightVector().GetSafeNormal();

	// [-1, 1]
	float forwardspeed = FVector::DotProduct(vel, forward);
	float rightspeed = FVector::DotProduct(vel, right);

	MoveForward(forwardspeed);
	MoveRight(rightspeed);
}

void UTankMovementComponent::Init(UTankTrack* left, UTankTrack* right)
{
	mLeftTrack = left;
	mRightTrack = right;
}

void UTankMovementComponent::MoveForward(float throttle)
{
	if (mLeftTrack == nullptr || mRightTrack == nullptr)
		return;

	mLeftTrack->SetThrottle(throttle);
	mRightTrack->SetThrottle(throttle);
}

void UTankMovementComponent::MoveRight(float throttle)
{
	if (mLeftTrack == nullptr || mRightTrack == nullptr)
		return;

	mLeftTrack->SetThrottle(throttle);
	mRightTrack->SetThrottle(-throttle);
}