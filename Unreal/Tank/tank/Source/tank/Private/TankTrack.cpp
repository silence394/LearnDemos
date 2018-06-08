// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"

void UTankTrack::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("UTankTrack DriveTrack11111111111"));
	Super::BeginPlay();

	OnComponentHit.AddDynamic(this, &UTankTrack::OnHit);
}

void UTankTrack::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HitComponent)
		DriveTrack();
}

void UTankTrack::SetThrottle(float throttle)
{
	mThrottle = FMath::Clamp<float>(mThrottle + throttle, -1.0f, 1.0f);
}

void UTankTrack::DriveTrack()
{
	FVector force = mTankMaxDriveForce * mThrottle * GetForwardVector();
	FVector forcelocation = GetComponentLocation();

	UPrimitiveComponent* rootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	rootComponent->AddForceAtLocation(force, forcelocation);

	mThrottle = 0.0f;
}