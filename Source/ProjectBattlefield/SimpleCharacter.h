// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "MainSpringArmComponent.h"
#include "MainCameraComponent.h"
#include "ActorInteractionInterface.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"

#include "SimpleCharacter.generated.h"

UCLASS()
class PROJECTBATTLEFIELD_API ASimpleCharacter : public ACharacter, public IActorInteractionInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	UMainSpringArmComponent* springArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	UMainCameraComponent* camera;

	UPROPERTY(BlueprintReadWrite, Category = "Possession Ability")
	APawn* possessorPawn;
	UPROPERTY(BlueprintReadWrite, Category = "Possession Ability")
	ASimpleCharacter* possessedCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputMappingContext* inputMappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaMove;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaRotateCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaSprint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaJump;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaPause;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInputs")
	UInputAction* iaPossessionAbility;

	UPROPERTY(BlueprintReadOnly, Category = "Simple Character")
	FTimerHandle possesDeacIATimerHandle;
	UPROPERTY(BlueprintReadOnly, Category = "Simple Character")
	FTimerHandle destroyActorTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Timeline: Possession Transition")
	UCurveFloat * possessionTransitionCurveFloat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timeline: Possession Transition")
	UTimelineComponent* possessionTransitionTimeline;

	UPROPERTY(BlueprintReadWrite, Category = "Timelines: Possession Transition")
	FVector lastSASocketOffset;
	UPROPERTY(BlueprintReadWrite, Category = "Timelines: Possession Transition")
	FVector lastActorLocation;
	UPROPERTY(BlueprintReadWrite, Category = "Timelines: Possession Transition")
	FRotator lastPossessorControlRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement: Walking")
	float maxWalkSpeedMain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character Movement: Walking")
	float maxSprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character Movement: Walking")
	float minSprintSpeed;
	UPROPERTY(BlueprintReadWrite, Category = "Character Movement: Flying")
	float maxFlySpeedMain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character Movement: Flying")
	float maxSprintSpeedWhenFlying;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Character Movement: Flying")
	float minSprintSpeedWhenFlying;
	UPROPERTY(BlueprintReadWrite, Category = "Timelines: Possession Transition")
	float lastCamFieldOfView;
	UPROPERTY(BlueprintReadWrite, Category = "Timelines: Possession Transition")
	float lastSATargetArmLength;


	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool bIsSprinting;
	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool bIsMovingWithKeyboard;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simple Character")
	bool bCanBePossessed;
	UPROPERTY(BlueprintReadWrite, Category = "Simple Character")
	bool bCanPossesByInputAction;
	UPROPERTY(BlueprintReadWrite, Category = "Simple Character")
	bool bIsPossessing;

	UPROPERTY(BlueprintReadWrite, Category = "EnhancedInputs")
	TMap<FString, bool> blockedInputsMap;

public:
	ASimpleCharacter();

protected:
	virtual void Restart() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Destroy();

	UFUNCTION()
	virtual void InputActionMove(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionMoveCompleted(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionRotateCamera(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void StartSprinting();
	UFUNCTION()
	virtual void StopSprinting();
	UFUNCTION()
	virtual void DeactivateCanPossesByInputAction();
	UFUNCTION()
	void MakeInvisible();
	UFUNCTION()
	void MakeVisible();
	UFUNCTION()
	virtual void InputActionSprint(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionJump(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionPause(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionPossessionAbilityStarted(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionPossessionAbilityCanceled(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionPossessionAbilityTriggered(const FInputActionInstance& Instance);

	UFUNCTION()
	void UpdatePossessionTransition(float alpha);
	UFUNCTION()
	void FinishedPossessionTransition();

public:
	UMainCameraComponent* GetCameraComponent();
	UMainSpringArmComponent* GetSpringArmComponent();
	APawn* GetPossessorPawn();
	bool CanBePossessed();

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;	
	UFUNCTION()
	virtual bool TakePossession(APawn* ogPossessorPawn, AController* possessorController);
	UFUNCTION()
	virtual void TakeDispossession(AController* playerController);
};
