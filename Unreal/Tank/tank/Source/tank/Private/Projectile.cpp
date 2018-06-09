// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"


// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mProjectileMoveCom = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	mProjectileMoveCom->bAutoActivate = false;

	mCollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("CollisionMesh"));
	mCollisionMesh->SetNotifyRigidBodyCollision(true);
	mCollisionMesh->SetVisibility(true);
	SetRootComponent(mCollisionMesh);

	mLaunchParticle = CreateDefaultSubobject<UParticleSystemComponent>(FName("Launch"));
	mLaunchParticle->AttachTo(RootComponent);
	mLaunchParticle->SetAutoActivate(false);

	mHitParticle = CreateDefaultSubobject<UParticleSystemComponent>(FName("Hit"));
	mHitParticle->AttachTo(RootComponent);
	mHitParticle->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	mCollisionMesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::LaunchProjectile(float speed)
{
	mProjectileMoveCom->SetVelocityInLocalSpace(FVector::ForwardVector * speed);
	mProjectileMoveCom->Activate();
	
	mLaunchParticle->SetActive(true);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	mHitParticle->SetActive(true);
}