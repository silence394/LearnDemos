// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUpBase.h"


// Sets default values
APickUpBase::APickUpBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mCollisionComponent = CreateDefaultSubobject<USphereComponent>(FName("CollisionArea"));
	mCollisionComponent->SetSphereRadius( 500.0f );
	SetRootComponent(mCollisionComponent);

	mMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
	mMeshComponent->SetSimulatePhysics(false);
	mMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void APickUpBase::BeginPlay()
{
	Super::BeginPlay();

	mCollisionComponent->bGenerateOverlapEvents = true;
	mCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APickUpBase::OnOverlap);
}

// Called every frame
void APickUpBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickUpBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UseItem(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("on OVERLAP!!"));
}

void APickUpBase::UseItem(AActor* actor)
{
	mCollisionComponent->bGenerateOverlapEvents = false;
	mCollisionComponent->SetSimulatePhysics(false);
	SetActorHiddenInGame(true);
}