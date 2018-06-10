// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPickUpHP.h"

void AMyPickUpHP::UseItem(AActor* actor)
{
	Super::UseItem(actor);
	AMyTank* tank = Cast<AMyTank>(actor);
	if (tank)
		tank->AddHP(mHPAdded);
}