// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"

void ATankPlayerController::SetPawn(APawn* inpawn)
{
	Super::SetPawn(inpawn);
	if (inpawn)
	{
		if (GetControlledTank())
			GetControlledTank()->mOnDeath.AddUniqueDynamic(this, &ATankPlayerController::OnControlTankDeath);
	}
}

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ATankPlayerController::Tick(float detaltime)
{
	AmiToTarget();
}

AMyTank* ATankPlayerController::GetControlledTank()
{
	return Cast<AMyTank>(GetPawn());
}

void ATankPlayerController::OnControlTankDeath()
{
	mMyTank = GetControlledTank();
	PlayerState->bIsSpectator = true;
	ChangeState(NAME_Spectating);
	GetWorldTimerManager().SetTimer(mTankRebornTimer, this, &ATankPlayerController::Reborn, mRebornTime, false);
}

void ATankPlayerController::AmiToTarget()
{
	if (GetPawn() == nullptr)
		return;

	FVector hit;
	if (GetSightRayHitLocation(hit))
		GetControlledTank()->FindComponentByClass<UTankAiming>()->AmiAt(hit);
}

bool ATankPlayerController::GetSightRayHitLocation(FVector& hitlocation)
{
	int32 vpsizex, vpsizey;
	GetViewportSize(vpsizex, vpsizey);
	FVector2D screenlocation = FVector2D(mCrossHairsX * vpsizex, mCrossHairsY * vpsizey);

	FVector worldlocation;
	FVector worlddirection;

	if (DeprojectScreenPositionToWorld(screenlocation.X, screenlocation.Y, worldlocation, worlddirection))
		return GetLookVectorHitLocation(worlddirection, hitlocation);
	else
		return false;
}

bool ATankPlayerController::GetLookVectorHitLocation(const FVector& lookdirection, FVector& outhitlocation)
{
	FVector start = PlayerCameraManager->GetCameraLocation();
	FVector end = start + lookdirection * mLineLength;

	FHitResult result;
	if (GetWorld()->LineTraceSingleByChannel(result, start, end, ECollisionChannel::ECC_Visibility) && result.Actor != GetControlledTank())
	{
		outhitlocation = result.Location;
		return true;
	}
	else
	{
		outhitlocation = FVector(0.0f);
		return false;
	}
}

void ATankPlayerController::Reborn()
{
	TArray<AActor*> playerstarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), playerstarts);
	AActor* borntarget = playerstarts[0];
	if (mMyTank == nullptr)
		return;

	mMyTank->SetHP(mMyTank->GetHPMax());
	mMyTank->SetActorTransform(borntarget->GetTransform());

	Possess(mMyTank);
	PlayerState->bIsSpectator = false;
	ChangeState(NAME_Playing);
}