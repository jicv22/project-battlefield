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

void UCombatStatics::ApplyDispossession(ASimpleCharacter* possessedCharacter, ASimpleCharacter* possessor)
{
	AController* controller = possessedCharacter->GetController();
	controller->UnPossess();
	possessor->TakeDispossession(controller);
}

void UCombatStatics::ApplyRecoil(FRotator recoil, ASimpleCharacter* character)
{
	character->TakeRecoil(recoil);
}
