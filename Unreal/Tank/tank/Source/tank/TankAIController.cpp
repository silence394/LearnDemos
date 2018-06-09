// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "TankAiming.h"

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();

	AMyTank* tank = GetControlledTank();
}

void ATankAIController::Tick(float deltatime)
{
	Super::Tick(deltatime);

	if (GetControlledTank())
	{
		MoveToActor(GetPlayerTank(), mAcceptanceRadius);

		UTankAiming* aiming =  GetControlledTank()->FindComponentByClass<UTankAiming>();
		aiming->AmiAt(GetPlayerTank()->GetActorLocation());
		if (aiming->mFireState == EFireState::FS_Locked)
			aiming->Fire();
	}
}

AMyTank* ATankAIController::GetControlledTank()
{
	return Cast<AMyTank>(GetPawn());
}

AMyTank* ATankAIController::GetPlayerTank()
{
	return Cast<AMyTank>(GetWorld()->GetFirstPlayerController()->GetPawn());
}
