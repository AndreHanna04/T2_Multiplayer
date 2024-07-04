// Fill out your copyright notice in the Description page of Project Settings.


#include "MFDPlayerController.h"

void AMFDPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	Pure_OnRepPosess(GetPawn());
}

void AMFDPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Pure_OnRepPosess(InPawn);
}
