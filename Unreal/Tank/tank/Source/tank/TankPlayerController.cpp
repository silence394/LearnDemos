// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AMyTank* tank = GetControlledTank();

	if (tank == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error: control Tank is null"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Control Tank is %s"), *tank->GetName());
	}
}

AMyTank* ATankPlayerController::GetControlledTank()
{
	return Cast<AMyTank>(GetPawn());
}


