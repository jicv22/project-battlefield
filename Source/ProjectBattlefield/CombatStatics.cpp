// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStatics.h"

bool UCombatStatics::ApplyPossession(ASimpleCharacter* possessor, AController* possessorController, ASimpleCharacter* possessedCharacter)
{
	possessorController->UnPossess();
	if (!possessedCharacter->TakePossession(possessorController, possessor->GetSpringArmComponent(), possessor->GetCamera()))
	{
		possessorController->Possess(possessor);
		return false;
	}
	return true;
}
