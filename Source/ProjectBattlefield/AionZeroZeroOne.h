// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OffensiveCharacter.h"
#include "AionZeroZeroOne.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBATTLEFIELD_API AAionZeroZeroOne : public AOffensiveCharacter
{
	GENERATED_BODY()
	
public:
	AAionZeroZeroOne();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void InputActionFireGun(const FInputActionInstance& Instance) override;
	virtual void InputActionSpecialAbility(const FInputActionInstance& Instance) override;
};
