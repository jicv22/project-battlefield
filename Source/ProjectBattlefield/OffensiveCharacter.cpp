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

	blockedInputsMap.Add(TEXT("Aim"), false);
	blockedInputsMap.Add(TEXT("FireGun"), false);
	blockedInputsMap.Add(TEXT("MeleeAttack"), false);
	blockedInputsMap.Add(TEXT("Reload"), false);
	blockedInputsMap.Add(TEXT("SpecialAbility"), false);

	camera->fieldOfViewAim = 50.f;
	springArm->targetArmLengthAim = 150.f;
	springArm->socketOffsetAim = FVector(0, 70, 50);

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

void AOffensiveCharacter::Restart()
{
	Super::Restart();
	bIsAiming = false;
}

void AOffensiveCharacter::UnPossessed()
{
	Aim(false);
	Super::UnPossessed();
}

void AOffensiveCharacter::Aim(bool shouldAim)
{
	if (shouldAim)
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
	if (blockedInputsMap[TEXT("Aim")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡Aim Input is Blocked!"));
	Aim(Instance.GetValue().Get<bool>());
}

void AOffensiveCharacter::InputActionMeleeAttack(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("MeleeAttack")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡MeleeAttack Input is Blocked!"));
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("MeleeAttack"));
}

void AOffensiveCharacter::InputActionFireGun(const FInputActionInstance& Instance)
{
	if (Instance.GetValue().Get<bool>() && !blockedInputsMap[TEXT("MeleeAttack")]) gun->StartShooting();
	else gun->StopShooting();
}

void AOffensiveCharacter::InputActionReload(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("Reload")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡Reload Input is Blocked!"));
	gun->StartReloading();
}

// to do: create childs and implement their own special ability 
void AOffensiveCharacter::InputActionSpecialAbility(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("SpecialAbility")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡SpecialAbility Input is Blocked!"));
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("SpecialAbility"));
}
