// Fill out your copyright notice in the Description page of Project Settings.


#include "AionZeroZeroOne.h"
#include "Kismet/KismetMathLibrary.h"

// to do: highlight when we are aiming to a stunned robot to let the user knows that he can possess it.
AAionZeroZeroOne::AAionZeroZeroOne()
{
}

void AAionZeroZeroOne::BeginPlay()
{
	Super::BeginPlay();
}

void AAionZeroZeroOne::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAionZeroZeroOne::InputActionFireGun(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, TEXT("¡Gun Fire is not available in this robot model!"));
}

void AAionZeroZeroOne::InputActionSpecialAbility(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("This character only have the possession ability ;v"));
}
