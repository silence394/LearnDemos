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
	if (tank == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error: AI control Tank is null"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Control Tank is %s, player tank is %s"), *tank->GetName(), *GetPlayerTank()->GetName());
	}
}