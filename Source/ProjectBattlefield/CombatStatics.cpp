// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStatics.h"
#include "Kismet/KismetMathLibrary.h"

bool UCombatStatics::ApplyPossession(ASimpleCharacter* possessor, ASimpleCharacter* possessedCharacter)
{
	AController* controller = possessor->GetController();
	controller->UnPossess();

	if (!possessedCharacter->TakePossession(possessor->GetPossessorPawn() ? possessor->GetPossessorPawn() : possessor, controller))
	{
		controller->Possess(possessor);
		return false;
	}

	return true;
}
