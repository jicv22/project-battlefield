// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStatics.h"

bool UCombatStatics::ApplyPossession(APawn* possessor, AController* possessorController, ASimpleCharacter* possessedCharacter)
{
	// to do: do here the logic to unposses th eplayer and validate if the other possession was succesful. Leave the rest of logic in the takePosessionMethod. Try to do it more modeluar or specific with the methods.
	possessorController->UnPossess(); 
	if (!possessedCharacter->TakePossession(possessorController))
	{
		possessorController->Possess(possessor);
		return false;
	}
	return true;
}
