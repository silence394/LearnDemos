// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"
#include "TankAiming.h"

void ATankAIController::SetPawn(APawn* inpawn)
{
	Super::SetPawn(inpawn);
	if (inpawn)
	{
		if (GetControlledTank())
			GetControlledTank()->mOnDeath.AddUniqueDynamic(this, &ATankAIController::OnControlTankDeath);
	}
}

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ATankAIController::Tick(float deltatime)
{
	Super::Tick(deltatime);

	if (GetControlledTank() && GetPlayerTank())
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

void ATankAIController::OnControlTankDeath()
{
	if (GetControlledTank())
		GetControlledTank()->DetachFromControllerPendingDestroy();
}