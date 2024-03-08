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

	UPROPERTY(BlueprintReadWrite, Category = "Timelines")
	UTimelineComponent* possessionCamTransitionTimeline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timelines")
	UCurveFloat* possessionCamTransitionFloatCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Simple Character")
	FTimerHandle possesDeacIATimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Simple Character")
	FRotator lastControlRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Timelines")
	FVector lastSpringArmLocation; // to do: create a variable similar to these for the camera fov, spring arm socket offset, spring arm target length, etc, for the possession transition
	UPROPERTY(BlueprintReadWrite, Category = "Timelines")
	FVector lastSpringArmSocketOffset;

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
	UPROPERTY(BlueprintReadWrite, Category = "Timelines")
	float lastSpringArmTargetArmLength;
	UPROPERTY(BlueprintReadWrite, Category = "Timelines")
	float lastCameraFOV;

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool bIsSprinting;
	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool bIsMovingWithKeyboard;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Simple Character")
	bool bCanBePossessed;
	UPROPERTY(BlueprintReadWrite, Category = "Simple Character")
	bool bCanPossesByInputAction;

	FOnTimelineFloat updateFunctionFloat;
	FOnTimelineEvent TimelineFinishedEvent;

public:
	ASimpleCharacter();

protected:
	virtual void Restart() override;
	virtual void UnPossessed() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	bool IsPossessedByAion();

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
	virtual void CamTransitionOnPossessionProgress(float value);
	UFUNCTION()
	virtual void CamTransitionOnPossessionFinished();

public:
	UCameraComponent* GetCamera();
	USpringArmComponent* GetSpringArmComponent();

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	virtual bool TakePossession(AController* possessorController, USpringArmComponent* possessorSpringArmComp, UCameraComponent* possessorCameraComp);
};
