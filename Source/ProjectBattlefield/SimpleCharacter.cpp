// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleCharacter.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"
#include "CombatStatics.h"

// to do: Make the character invisible when camera is too close
ASimpleCharacter::ASimpleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	possessionCamTransitionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PossessionCamTransitionTimeline"));
	possessionCamTransitionFloatCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("PossessionCamTransitionFloatCurve"));
	springArm = CreateDefaultSubobject<UMainSpringArmComponent>(TEXT("SpringArm"));
	camera = CreateDefaultSubobject<UMainCameraComponent>(TEXT("Camera"));
	inputMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("InputMappingContext"));
	iaMove = CreateDefaultSubobject<UInputAction>(TEXT("InputActionMove"));
	iaRotateCamera = CreateDefaultSubobject<UInputAction>(TEXT("InputActionRotateCamera"));
	iaSprint = CreateDefaultSubobject<UInputAction>(TEXT("InputActionSprint"));
	iaJump = CreateDefaultSubobject<UInputAction>(TEXT("InputActionJump"));
	iaPause = CreateDefaultSubobject<UInputAction>(TEXT("InputActionPause"));
	iaPossessionAbility = CreateDefaultSubobject<UInputAction>(TEXT("InputActionPossessionAbility"));

	springArm->SetupAttachment(GetCapsuleComponent());
	camera->SetupAttachment(springArm);

	lastControlRotation = FRotator(0);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	maxSprintSpeed = 600.f;
	minSprintSpeed = 300.f;
	GetCharacterMovement()->MaxFlySpeed = 300.f;
	maxSprintSpeedWhenFlying = 600.f;
	minSprintSpeedWhenFlying = 300.f;
	lastPossessorCamLocation = FVector(0);

	bIsSprinting = false;
	bIsMovingWithKeyboard = false;
	bCanBePossessed = true;
	bCanPossesByInputAction = false;


	camera->FieldOfView = 90.f;
	camera->fieldOfViewAim = 50.f;
	springArm->TargetArmLength = 150.f;
	springArm->targetArmLengthAim = 150.f;
	springArm->SocketOffset = FVector(0, 70, 50);
	springArm->socketOffsetAim = FVector(0, 70, 50);
	springArm->bUsePawnControlRotation = true;
	springArm->bInheritYaw = true;
	springArm->bInheritPitch = true;
	springArm->bInheritRoll = false;
	springArm->bEnableCameraLag = true;
	springArm->CameraLagMaxDistance = 50.f;
	springArm->CameraLagSpeed = 20.f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCapsuleRadius(25.f);
	GetCapsuleComponent()->SetCapsuleHalfHeight(50.f);
}

void ASimpleCharacter::Restart()
{
	Super::Restart();
	GetController()->SetControlRotation(lastControlRotation);
}

void ASimpleCharacter::UnPossessed()
{
	Super::UnPossessed();
	// to do: Do something when unPossessed.
}

void ASimpleCharacter::BeginPlay()
{
	Super::BeginPlay();

	maxWalkSpeedMain = GetCharacterMovement()->MaxWalkSpeed;
	maxFlySpeedMain = GetCharacterMovement()->MaxFlySpeed;

	updateFunctionFloat.BindDynamic(this, &ASimpleCharacter::CamTransitionOnPossessionProgress);
	if (possessionCamTransitionFloatCurve)
	{
		possessionCamTransitionTimeline->AddInterpFloat(possessionCamTransitionFloatCurve, updateFunctionFloat);
	}
	TimelineFinishedEvent.BindUFunction(this, FName("CamTransitionOnPossessionFinished"));
	possessionCamTransitionTimeline->SetTimelineFinishedFunc(TimelineFinishedEvent);
}

void ASimpleCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float characterVelocity = GetVelocity().Length();
	bool isUnderMinSprintSpeed = GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking && characterVelocity < minSprintSpeed;
	bool isUnderMinSprintSpeedWhenFlying = GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying && characterVelocity < minSprintSpeedWhenFlying;
	if (bIsSprinting && (isUnderMinSprintSpeed || isUnderMinSprintSpeedWhenFlying) && !bIsMovingWithKeyboard) StopSprinting();
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
				Input->BindAction(iaPossessionAbility, ETriggerEvent::Started, this, &ASimpleCharacter::InputActionPossessionAbilityStarted);
				Input->BindAction(iaPossessionAbility, ETriggerEvent::Canceled, this, &ASimpleCharacter::InputActionPossessionAbilityCanceled);
				Input->BindAction(iaPossessionAbility, ETriggerEvent::Triggered, this, &ASimpleCharacter::InputActionPossessionAbilityTriggered);
				// to do: cancel = posses......hold completed = unposses
				// to do: add a function to allow the user to posses another character or to unposses the curretn character.
				// to do: also add a variable to knwo if the player/character can possess another one. this can be use with a time when key is pressed to allow a good functionality when the player cancel the key holding.
			}
		}
	}
}

bool ASimpleCharacter::IsPossessedByAion()
{
	// to do: change this and verify when is possessed by Aion with AI or by Aion Player....
	return IsPlayerControlled();
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

	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking && !isMovingHorizontally) bIsMovingWithKeyboard = false;
	else bIsMovingWithKeyboard = true;
}

void ASimpleCharacter::InputActionMoveCompleted(const FInputActionInstance& Instance)
{
	bIsMovingWithKeyboard = false;
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
	bIsSprinting = true;
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
	bIsSprinting = false;
}

void ASimpleCharacter::DeactivateCanPossesByInputAction()
{
	bCanPossesByInputAction = false;
}

void ASimpleCharacter::InputActionSprint(const FInputActionInstance& Instance)
{
	if (bIsSprinting)
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

void ASimpleCharacter::InputActionPossessionAbilityStarted(const FInputActionInstance& Instance)
{
	bCanPossesByInputAction = true;
	GetWorldTimerManager().SetTimer(possesDeacIATimerHandle, this, &ASimpleCharacter::DeactivateCanPossesByInputAction, 0.2f, false);
}

void ASimpleCharacter::InputActionPossessionAbilityCanceled(const FInputActionInstance& Instance)
{
	// to do: Destroy the "Aion" robot or the previeous robot when the possession have succeded.
	// to do: check why the unpossessed character got frezeed.
	if (!IsPossessedByAion() || !bCanPossesByInputAction) return;
	
	bCanPossesByInputAction = false;
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("Trying to posses something"));

	FVector forwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());

	FHitResult hitResult;
	FVector traceStartLocation = camera->GetComponentLocation() + (forwardVector * springArm->TargetArmLength);
	FVector traceEndLocation = traceStartLocation + (forwardVector * 500); // to do: set a parameter for the possession range
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	GetWorld()->SweepSingleByChannel(hitResult, traceStartLocation, traceEndLocation, FQuat::FindBetween(traceStartLocation, traceEndLocation), ECollisionChannel::ECC_WorldDynamic, FCollisionShape::MakeSphere(20.f), queryParams);

	ASimpleCharacter* hitCharacter = Cast<ASimpleCharacter>(hitResult.GetActor());
	if (hitResult.bBlockingHit && hitCharacter)
	{
		lastControlRotation = GetControlRotation();
		if (UCombatStatics::ApplyPossession(this, GetController(), hitCharacter))
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("Possession Succeded"));
		}
	}

	/*Debug*/
	DrawDebugSphere(GetWorld(), hitResult.Location, 14.5f, 12, FColor::Blue, false, 2.f);
	DrawDebugLine(GetWorld(), traceStartLocation, traceEndLocation, hitResult.bBlockingHit ? FColor::Green : FColor::Red, false, 2.f);
	/*Debug*/
}

void ASimpleCharacter::InputActionPossessionAbilityTriggered(const FInputActionInstance& Instance)
{
	// to do: Create the unpossess functionality...
}

void ASimpleCharacter::CamTransitionOnPossessionProgress(float value)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::SanitizeFloat(value));
	FVector newCamPosition = FMath::Lerp(lastPossessorCamLocation, FVector(0), value);
	springArm->SetRelativeLocation(newCamPosition);
	// to do: Improve to interpolate possessor and possessed's FOV, socket offset and all that kind of things
}

void ASimpleCharacter::CamTransitionOnPossessionFinished()
{
	springArm->bEnableCameraLag = true;
}

UCameraComponent* ASimpleCharacter::GetCamera()
{
	return camera;
}

USpringArmComponent* ASimpleCharacter::GetSpringArmComponent()
{
	return springArm;
}

float ASimpleCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); // to do: check if we should or shouldn't use this.
	if (!CanBeDamaged()) return 0.f;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, TEXT("I have been damaged by: ") + EventInstigator->GetActorNameOrLabel() + TEXT(" For: ") + FString::SanitizeFloat(DamageAmount) + TEXT(" Damage Points"));

	return DamageAmount;
}

bool ASimpleCharacter::TakePossession(AController* possessorController, USpringArmComponent* possessorSpringArmComp)
{
	if (!bCanBePossessed) return false;

	lastControlRotation = possessorController->GetControlRotation();
	// to do: change possessorSpringArmComp for just a FVector possessorSpringArmCompLocation or something like that, then we could be requiring something like the possessor's FOV, spring arm socket offset, etc
	springArm->SetWorldLocation(possessorSpringArmComp->GetComponentLocation());
	lastPossessorCamLocation = springArm->GetRelativeLocation();
	springArm->bEnableCameraLag = false;

	possessorController->Possess(this);	
	possessionCamTransitionTimeline->PlayFromStart();
	
	return true;
}
