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
// to do: translate the camera position (socket offset) when a wall is close from right side.
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
	iaPossessionAbility = CreateDefaultSubobject<UInputAction>(TEXT("InputActionPossessionAbility"));
	possessionTransitionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PossessionTransitionTimeline"));
	camRecoilTransitionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CamRecoilTransitionTimeline"));
	possessionTransitionCurveFloat = CreateDefaultSubobject<UCurveFloat>(TEXT("PossessionTransitionCurveFloat"));
	camRecoilTransitionCurveFloat = CreateDefaultSubobject<UCurveFloat>(TEXT("CamRecoilTransitionCurveFloat"));


	springArm->SetupAttachment(GetCapsuleComponent());
	camera->SetupAttachment(springArm);

	blockedInputsMap.Add(TEXT("Move"), false);
	blockedInputsMap.Add(TEXT("RotateCamera"), false);
	blockedInputsMap.Add(TEXT("Sprint"), false);
	blockedInputsMap.Add(TEXT("Jump"), false);
	blockedInputsMap.Add(TEXT("PossessionAbility"), false);

	lastSASocketOffset = FVector(0);
	lastActorLocation = FVector(0);

	lastPossessorControlRotation = FRotator(0);
	camRecoilCtrlRotationStart = FRotator(0);
	camRecoilCtrlRotationEnd = FRotator(0);

	maxWalkSpeedMain = 0.f;
	maxFlySpeedMain = 0.f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	maxSprintSpeed = 600.f;
	minSprintSpeed = 300.f;
	GetCharacterMovement()->MaxFlySpeed = 300.f;
	maxSprintSpeedWhenFlying = 600.f;
	minSprintSpeedWhenFlying = 300.f;
	lastCamFieldOfView = 0.f;
	lastSATargetArmLength = 0.f;

	bIsSprinting = false;
	bIsMovingWithKeyboard = false;
	bCanBePossessed = true;
	bCanPossesByInputAction = false;
	bIsPossessing = false;

	camera->FieldOfView = 70.f;
	springArm->TargetArmLength = 150.f;
	springArm->SocketOffset = FVector(0, 70, 50);
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

	possessorPawn = nullptr;
	possessedCharacter = nullptr;
}

void ASimpleCharacter::Restart()
{
	Super::Restart();
	GetController()->SetControlRotation(lastPossessorControlRotation);

}

void ASimpleCharacter::BeginPlay()
{
	Super::BeginPlay();

	maxWalkSpeedMain = GetCharacterMovement()->MaxWalkSpeed;
	maxFlySpeedMain = GetCharacterMovement()->MaxFlySpeed;

	if (possessionTransitionCurveFloat)
	{

		FOnTimelineEvent possessionTransitionFinishedEvent;
		possessionTransitionFinishedEvent.BindUFunction(this, FName("FinishedPossessionTransition"));
		possessionTransitionTimeline->SetTimelineFinishedFunc(possessionTransitionFinishedEvent);

		FOnTimelineFloat updatePossessionTransitionTrack;
		updatePossessionTransitionTrack.BindDynamic(this, &ASimpleCharacter::UpdatePossessionTransition);
		possessionTransitionTimeline->AddInterpFloat(possessionTransitionCurveFloat, updatePossessionTransitionTrack);
	}
	if (camRecoilTransitionCurveFloat)
	{
		FOnTimelineFloat updateCamRecoilTransitionTrack;
		updateCamRecoilTransitionTrack.BindDynamic(this, &ASimpleCharacter::UpdateCamRecoilTransition);
		camRecoilTransitionTimeline->AddInterpFloat(camRecoilTransitionCurveFloat, updateCamRecoilTransitionTrack);
	}

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
			}
		}
	}
}

void ASimpleCharacter::Destroy()
{
	Super::Destroy();
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Actor Destroyed"));
}

void ASimpleCharacter::InputActionMove(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("Move")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡Movement Input is Blocked!"));

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
	if (blockedInputsMap[TEXT("RotateCamera")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡RotateCamera Input is Blocked!"));
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

void ASimpleCharacter::MakeInvisible()
{
	GetRootComponent()->SetVisibility(false, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASimpleCharacter::MakeVisible()
{
	GetRootComponent()->SetVisibility(true, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ASimpleCharacter::InputActionSprint(const FInputActionInstance& Instance)
{
	if (!bIsSprinting && !blockedInputsMap[TEXT("Sprint")])
	{
		StartSprinting();
	}
	else
	{
		StopSprinting();
	}
}

void ASimpleCharacter::InputActionJump(const FInputActionInstance& Instance)
{
	if (Instance.GetValue().Get<bool>() && !blockedInputsMap[TEXT("Jump")]) Jump();
	else StopJumping();
}

void ASimpleCharacter::InputActionPause(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("Pause")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡Pause Input is Blocked!"));
	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("inputActionPause"));
}

void ASimpleCharacter::InputActionPossessionAbilityStarted(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("PossessionAbility")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡PossessionAbility Input is Blocked!"));
	bCanPossesByInputAction = true;
	GetWorldTimerManager().SetTimer(possesDeacIATimerHandle, this, &ASimpleCharacter::DeactivateCanPossesByInputAction, 0.25f, false);
}

void ASimpleCharacter::InputActionPossessionAbilityCanceled(const FInputActionInstance& Instance)
{
	if (blockedInputsMap[TEXT("PossessionAbility")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡PossessionAbility Input is Blocked!"));
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("Trying to posses something"));

	if (!bCanPossesByInputAction) return GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("Possession Time Range already expired"));
	bCanPossesByInputAction = false;

	FVector forwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	FVector traceStart = GetCameraComponent()->GetComponentLocation() + (forwardVector * GetSpringArmComponent()->TargetArmLength);
	FVector traceEnd = traceStart + (forwardVector * 750);

	FHitResult hitResult;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECollisionChannel::ECC_Pawn, queryParams);

	/*Debug*/
	DrawDebugLine(GetWorld(), traceStart, !hitResult.bBlockingHit ? traceEnd : hitResult.Location, FColor::Red, false, 5.f);
	/*Debug*/

	if (!hitResult.bBlockingHit) return GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("Nothing Reached :("));
	
	/*Debug*/
	DrawDebugSphere(GetWorld(), hitResult.Location, 5.f, 12, FColor::Green, false, 5.f);
	/*Debug*/

	ASimpleCharacter* characterHit = Cast<ASimpleCharacter>(hitResult.GetActor());
	if(!characterHit || !characterHit->CanBePossessed()) return GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("This actor can't be possessed"));

	GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Cyan, TEXT("¡Yeah! We found a character to possess"));

	MakeInvisible();

	possessedCharacter = characterHit;
	SetActorRotation(possessedCharacter->GetActorRotation());
	lastCamFieldOfView = camera->FieldOfView;
	lastSASocketOffset = springArm->SocketOffset;
	lastSATargetArmLength = springArm->TargetArmLength;
	lastActorLocation = GetActorLocation();

	for (auto& inputState : blockedInputsMap)
	{
		if (inputState.Key != TEXT("RotateCamera") && inputState.Key != TEXT("Pause")) blockedInputsMap[inputState.Key] = true;
	}

	bIsPossessing = true;
	possessionTransitionTimeline->PlayFromStart();
}

void ASimpleCharacter::InputActionPossessionAbilityTriggered(const FInputActionInstance& Instance)
{
	if (!possessorPawn  || blockedInputsMap[TEXT("PossessionAbility")]) return GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("¡Can't use Dispossession Ability!"));

	MakeInvisible();

	possessedCharacter = Cast<ASimpleCharacter>(possessorPawn);

	possessorPawn->SetActorRotation(GetActorRotation());
	possessorPawn->SetActorLocation(GetActorLocation()+FVector(0,0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*0.5f));
	lastActorLocation = GetActorLocation();
	lastCamFieldOfView = camera->FieldOfView;
	lastPossessorControlRotation = GetControlRotation();
	lastSASocketOffset = springArm->SocketOffset;
	lastSATargetArmLength = springArm->TargetArmLength;

	possessionTransitionTimeline->PlayFromStart();
}

void ASimpleCharacter::UpdatePossessionTransition(float alpha)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::SanitizeFloat(alpha));
	float newFieldOfView = FMath::Lerp(lastCamFieldOfView, possessedCharacter->GetCameraComponent()->fieldOfViewMain, alpha);
	float newTargetArmLength = FMath::Lerp(lastSATargetArmLength, possessedCharacter->GetSpringArmComponent()->targetArmLengthMain, alpha);
	FVector newSocketOffset = FMath::Lerp(lastSASocketOffset, possessedCharacter->GetSpringArmComponent()->socketOffsetMain, alpha);
	FVector newActorLocation = FMath::Lerp(lastActorLocation, possessedCharacter->GetActorLocation(), alpha);

	camera->FieldOfView = newFieldOfView;
	springArm->TargetArmLength = newTargetArmLength;
	springArm->SocketOffset = newSocketOffset;
	SetActorLocation(newActorLocation, true);
}

void ASimpleCharacter::FinishedPossessionTransition()
{
	if(bIsPossessing) UCombatStatics::ApplyPossession(this, possessedCharacter);
	else UCombatStatics::ApplyDispossession(this, Cast<ASimpleCharacter>(possessorPawn));

	if (GetPossessorPawn())
	{
		GetWorldTimerManager().ClearTimer(destroyActorTimerHandle);
		GetWorldTimerManager().SetTimer(destroyActorTimerHandle, this, &ASimpleCharacter::Destroy, 3, false);
	}
	else
	{
		for (auto& inputState : blockedInputsMap)
		{
			blockedInputsMap[inputState.Key] = false;
		}
		bIsPossessing = false;
	}
}

void ASimpleCharacter::UpdateCamRecoilTransition(float alpha)
{
	GetController()->SetControlRotation(GetControlRotation()+(camRecoilCtrlRotationEnd*alpha));
}

UMainCameraComponent* ASimpleCharacter::GetCameraComponent()
{
	return camera;
}

UMainSpringArmComponent* ASimpleCharacter::GetSpringArmComponent()
{
	return springArm;
}

APawn* ASimpleCharacter::GetPossessorPawn()
{
	return possessorPawn;
}

bool ASimpleCharacter::CanBePossessed()
{
	return bCanBePossessed;
}

float ASimpleCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); // to do: check if we should or shouldn't use this.
	if (!CanBeDamaged()) return 0.f;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, TEXT("I have been damaged by: ") + EventInstigator->GetActorNameOrLabel() + TEXT(" For: ") + FString::SanitizeFloat(DamageAmount) + TEXT(" Damage Points"));

	return DamageAmount;
}

bool ASimpleCharacter::TakePossession(APawn* ogPossessorPawn, AController* possessorController)
{
	if (!bCanBePossessed) return false;

	possessorPawn = ogPossessorPawn;

	lastPossessorControlRotation = possessorController->GetControlRotation();

	possessorController->Possess(this);

	return true;
}

void ASimpleCharacter::TakeDispossession(AController* playerController)
{
	possessedCharacter = nullptr;
	lastPossessorControlRotation = playerController->GetControlRotation();

	MakeVisible();

	playerController->Possess(this);
}

void ASimpleCharacter::TakeRecoil(FRotator recoil)
{
	camRecoilCtrlRotationStart = GetControlRotation();
	camRecoilCtrlRotationEnd = recoil;
	camRecoilTransitionTimeline->PlayFromStart();
}
