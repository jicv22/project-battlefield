// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "MainSpringArmComponent.h"
#include "MainCameraComponent.h"
#include "ActorInteractionInterface.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"

#include "SimpleCharacter.generated.h"

UCLASS()
class PROJECTBATTLEFIELD_API ASimpleCharacter : public ACharacter, public IActorInteractionInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	UMainSpringArmComponent* springArm;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	UMainCameraComponent* camera;

protected:
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

	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool isSprinting;
	UPROPERTY(BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool isMovingWithKeyboard;
	

public:
	ASimpleCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void InputActionMove(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionMoveCompleted(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionRotateCamera(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void StopSprinting();
	UFUNCTION()
	virtual void StartSprinting();
	UFUNCTION()
	virtual void InputActionSprint(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionJump(const FInputActionInstance& Instance);
	UFUNCTION()
	virtual void InputActionPause(const FInputActionInstance& Instance);

public:
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
