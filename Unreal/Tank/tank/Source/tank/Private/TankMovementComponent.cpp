// Fill out your copyright notice in the Description page of Project Settings.

#include "TankMovementComponent.h"

void UTankMovementComponent::Init(UTankTrack* left, UTankTrack* right)
{
	if (left == nullptr || right == nullptr)
		return;

	mLeftTrack = left;
	mRightTrack = right;
}

void UTankMovementComponent::MoveForward(float throttle)
{
	mLeftTrack->SetThrottle(throttle);
	mRightTrack->SetThrottle(throttle);
}

void UTankMovementComponent::MoveRight(float throttle)
{
	mLeftTrack->SetThrottle(throttle);
	mRightTrack->SetThrottle(-throttle);
}