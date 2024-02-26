// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleCharacter.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"

// to do: Make the character invisible when camera is too close
ASimpleCharacter::ASimpleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	springArm = CreateDefaultSubobject<UMainSpringArmComponent>(TEXT("SpringArm"));
	camera = CreateDefaultSubobject<UMainCameraComponent>(TEXT("Camera"));
	inputMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("InputMappingContext"));
	iaMove = CreateDefaultSubobject<UInputAction>(TEXT("InputActionMove"));
	iaRotateCamera = CreateDefaultSubobject<UInputAction>(TEXT("InputActionRotateCamera"));
	iaSprint = CreateDefaultSubobject<UInputAction>(TEXT("InputActionSprint"));
	iaJump = CreateDefaultSubobject<UInputAction>(TEXT("InputActionJump"));
	iaPause = CreateDefaultSubobject<UInputAction>(TEXT("InputActionPause"));

	springArm->SetupAttachment(GetCapsuleComponent());
	camera->SetupAttachment(springArm);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	maxFlySpeedMain = 0.f;
	maxSprintSpeed = 600.f;
	minSprintSpeed = 300.f;
	GetCharacterMovement()->MaxFlySpeed = 300.f;
	maxSprintSpeedWhenFlying = 600.f;
	minSprintSpeedWhenFlying = 300.f;

	isSprinting = false;
	isMovingWithKeyboard = false;

	camera->FieldOfView = 90;
	camera->fieldOfViewAim = 50;
	springArm->TargetArmLength = 150;
	springArm->targetArmLengthAim = 150;
	springArm->SocketOffset = FVector(0, 70, 50);
	springArm->socketOffsetAim = FVector(0, 70, 50);
	springArm->bUsePawnControlRotation = true;
	springArm->bInheritYaw = true;
	springArm->bInheritPitch = true;
	springArm->bInheritRoll = false;
	springArm->bEnableCameraLag = true;
	springArm->CameraLagMaxDistance = 50;
	springArm->CameraLagSpeed = 20;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCapsuleRadius(25.f);
	GetCapsuleComponent()->SetCapsuleHalfHeight(50.f);
}

void ASimpleCharacter::BeginPlay()
{
	Super::BeginPlay();

	maxWalkSpeedMain = GetCharacterMovement()->MaxWalkSpeed;
	maxFlySpeedMain = GetCharacterMovement()->MaxFlySpeed;
}

void ASimpleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float characterVelocity = GetVelocity().Length();
	bool isUnderMinSprintSpeed = GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking && characterVelocity < minSprintSpeed;
	bool isUnderMinSprintSpeedWhenFlying = GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying && characterVelocity < minSprintSpeedWhenFlying;
	if (isSprinting && (isUnderMinSprintSpeed || isUnderMinSprintSpeedWhenFlying) && !isMovingWithKeyboard) StopSprinting();
}

void ASimpleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(inputMappingContext, 0);

			if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
			{
				Input->BindAction(iaMove, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionMove);
				Input->BindAction(iaMove, ETriggerEvent::Completed, this, &ASimpleCharacter::InputActionMoveCompleted);
				Input->BindAction(iaRotateCamera, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionRotateCamera);
				Input->BindAction(iaSprint, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionSprint);
				Input->BindAction(iaJump, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionJump);
				Input->BindAction(iaPause, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionPause);
			}
		}
	}
}

void ASimpleCharacter::InputActionMove(const FInputActionInstance& Instance)
{
	FVector3d axisValue = Instance.GetValue().Get<FVector3d>();
	FVector forwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0));
	FVector rightVector = UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0));

	bool isMovingHorizontally = axisValue.X != 0 || axisValue.Y != 0;
	if (!isMovingHorizontally) GetCharacterMovement()->bOrientRotationToMovement = false;
	else GetCharacterMovement()->bOrientRotationToMovement = true;

	AddMovementInput(rightVector, axisValue.X);
	AddMovementInput(forwardVector, axisValue.Y);
	AddMovementInput(FVector(0, 0, 1), axisValue.Z);

	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking && !isMovingHorizontally) isMovingWithKeyboard = false;
	else isMovingWithKeyboard = true;
}

void ASimpleCharacter::InputActionMoveCompleted(const FInputActionInstance& Instance)
{
	isMovingWithKeyboard = false;
}

void ASimpleCharacter::InputActionRotateCamera(const FInputActionInstance& Instance)
{
	FVector2D axisValues = Instance.GetValue().Get<FVector2D>();

	AddControllerYawInput(axisValues.X * 0.5f);
	AddControllerPitchInput(axisValues.Y * 0.5f);
}

void ASimpleCharacter::StartSprinting()
{
	switch (GetCharacterMovement()->MovementMode)
	{
		case EMovementMode::MOVE_Flying:
			GetCharacterMovement()->MaxFlySpeed = maxSprintSpeedWhenFlying;
			break;
		default:
			GetCharacterMovement()->MaxWalkSpeed = maxSprintSpeed;
			break;
	}
	isSprinting = true;
}

void ASimpleCharacter::StopSprinting()
{
	switch (GetCharacterMovement()->MovementMode)
	{
	case EMovementMode::MOVE_Flying:
		GetCharacterMovement()->MaxFlySpeed = maxFlySpeedMain;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = maxWalkSpeedMain;
		break;
	}
	isSprinting = false;
}

void ASimpleCharacter::InputActionSprint(const FInputActionInstance& Instance)
{
	if (isSprinting)
	{
		StopSprinting();
	}
	else
	{
		StartSprinting();
	}
}

void ASimpleCharacter::InputActionJump(const FInputActionInstance& Instance)
{
	if (Instance.GetValue().Get<bool>()) Jump();
	else StopJumping();
}

void ASimpleCharacter::InputActionPause(const FInputActionInstance& Instance)
{
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("inputActionPause"));
}

float ASimpleCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, TEXT("I have been damaged by: ") + EventInstigator->GetActorNameOrLabel() + TEXT(" For: ") + FString::SanitizeFloat(DamageAmount) + TEXT(" Damage Points"));
	return DamageAmount;
}
