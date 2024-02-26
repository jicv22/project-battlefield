// Fill out your copyright notice in the Description page of Project Settings.

#include "OffensiveCharacter.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Kismet/KismetMathLibrary.h"

AOffensiveCharacter::AOffensiveCharacter()
{
	iaAim = CreateDefaultSubobject<UInputAction>(TEXT("InputActionAim"));
	iaFireGun = CreateDefaultSubobject<UInputAction>(TEXT("InputActionFireGun"));
	iaMeleeAttack = CreateDefaultSubobject<UInputAction>(TEXT("InputActionMeleeAttack"));
	iaReload = CreateDefaultSubobject<UInputAction>(TEXT("InputActionReload"));
	iaSpecialAbility = CreateDefaultSubobject<UInputAction>(TEXT("InputActionSpecialAbility"));
	AimTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimTimeline"));
	AimTimelineCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("AimTimelineCurve"));
	gun = CreateDefaultSubobject<UGunComponent>(TEXT("Gun"));

	bIsAiming = false;
}

void AOffensiveCharacter::BeginPlay()
{
	Super::BeginPlay();

	FOnTimelineFloat onUpdate;
	onUpdate.BindDynamic(this, &AOffensiveCharacter::AimTransitionUpdate);

	if (AimTimelineCurve)
	{
		AimTimeline->AddInterpFloat(AimTimelineCurve, onUpdate);
	}
}

void AOffensiveCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(iaAim, ETriggerEvent::Triggered, this, &AOffensiveCharacter::InputActionAim);
		Input->BindAction(iaFireGun, ETriggerEvent::Triggered, this, &AOffensiveCharacter::InputActionFireGun);
		Input->BindAction(iaMeleeAttack, ETriggerEvent::Triggered, this, &AOffensiveCharacter::InputActionMeleeAttack);
		Input->BindAction(iaReload, ETriggerEvent::Triggered, this, &AOffensiveCharacter::InputActionReload);
		Input->BindAction(iaSpecialAbility, ETriggerEvent::Triggered, this, &AOffensiveCharacter::InputActionSpecialAbility);
	}
}

void AOffensiveCharacter::AimTransitionUpdate(float value)
{
	float newTargetArmLegnth = FMath::Lerp(springArm->targetArmLengthMain, springArm->targetArmLengthAim, value);
	float newFielOfView = FMath::Lerp(camera->fieldOfViewMain, camera->fieldOfViewAim, value);
	FVector newSocketOffset = FMath::Lerp(springArm->socketOffsetMain, springArm->socketOffsetAim, value);
	springArm->TargetArmLength = newTargetArmLegnth;
	springArm->SocketOffset = newSocketOffset;
	camera->FieldOfView = newFielOfView;
}

void AOffensiveCharacter::InputActionAim(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("inputActionAim"));
	if (Instance.GetValue().Get<bool>())
	{
		AimTimeline->Play();
		bIsAiming = true;
		springArm->CameraLagMaxDistance = 10;
	}
	else
	{
		AimTimeline->Reverse();
		bIsAiming = false;
		springArm->CameraLagMaxDistance = 50;
	}
}

void AOffensiveCharacter::InputActionMeleeAttack(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("MeleeAttack"));
}

void AOffensiveCharacter::InputActionFireGun(const FInputActionInstance& Instance)
{
	if (Instance.GetValue().Get<bool>()) gun->StartShooting();
	else gun->StopShooting();
}

void AOffensiveCharacter::InputActionReload(const FInputActionInstance& Instance)
{
	gun->StartReloading();
}

// to do: create the possession special ability functionality here to has it like the special ability by default.
void AOffensiveCharacter::InputActionSpecialAbility(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("SpecialAbility"));
}
