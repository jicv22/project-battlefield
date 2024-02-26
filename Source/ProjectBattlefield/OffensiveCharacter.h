// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SimpleCharacter.h"
#include "GunComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/TimelineComponent.h"
#include "OffensiveCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBATTLEFIELD_API AOffensiveCharacter : public ASimpleCharacter
{
	GENERATED_BODY()
	
public:

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaAim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaFireGun;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaMeleeAttack;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaReload;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaSpecialAbility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timelines")
	UTimelineComponent* AimTimeline;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timelines")
	UCurveFloat* AimTimelineCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	UGunComponent* gun;
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAiming;

public:
	AOffensiveCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void AimTransitionUpdate(float value);
	UFUNCTION()
	virtual void InputActionAim(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionMeleeAttack(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionFireGun(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionReload(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionSpecialAbility(const FInputActionInstance& Instance);
};
