// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleCharacter.h"

#include "CombatStatics.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBATTLEFIELD_API UCombatStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static bool ApplyPossession(ASimpleCharacter* possessor, ASimpleCharacter* possessedCharacter);
};
