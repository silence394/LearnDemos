// Fill out your copyright notice in the Description page of Project Settings.

#include "TankAIController.h"


AMyTank* ATankAIController::GetControlledTank()
{
	return Cast<AMyTank>(GetPawn());
}

AMyTank* ATankAIController::GetPlayerTank()
{
	return Cast<AMyTank>(GetWorld()->GetFirstPlayerController()->GetPawn());
}

void ATankAIController::BeginPlay()
{
	Super::BeginPlay();

	AMyTank* tank = GetControlledTank();
}