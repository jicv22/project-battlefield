// Fill out your copyright notice in the Description page of Project Settings.


#include "AionZeroZeroOne.h"
#include "Kismet/KismetMathLibrary.h"

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
	// to do: use a type of capsule where the player/camera is seen. Create something to remark the target and when we press the ability button we'll posses that robot if it's valid.
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("InputActionSpecialAbility"));
}
