// Fill out your copyright notice in the Description page of Project Settings.

#include "TankTrack.h"

void UTankTrack::BeginPlay()
{
	Super::BeginPlay();

	OnComponentHit.AddDynamic(this, &UTankTrack::OnHit);
}

void UTankTrack::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	DriveTrack();
	ApplySideForce();

	mThrottle = 0.0f;
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
}

void UTankTrack::ApplySideForce()
{
	UPrimitiveComponent* rootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	FVector velocity = rootComponent->GetComponentVelocity();
	FVector rightvec = rootComponent->GetRightVector();

	float sidespeed = FVector::DotProduct(velocity, rightvec);

	FVector sideacc = sidespeed / GetWorld()->DeltaTimeSeconds * rightvec * -1.0f;
	FVector correctforce = rootComponent->GetMass() * sideacc / 2.0f;
	rootComponent->AddForce(correctforce);
}