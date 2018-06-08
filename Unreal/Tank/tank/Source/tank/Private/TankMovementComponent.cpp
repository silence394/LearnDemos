// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"

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