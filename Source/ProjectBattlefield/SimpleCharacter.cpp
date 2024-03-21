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
	lastSpringArmLocation = FVector(0);
	lastSASocketOffset = FVector(0);

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	maxSprintSpeed = 600.f;
	minSprintSpeed = 300.f;
	GetCharacterMovement()->MaxFlySpeed = 300.f;
	maxSprintSpeedWhenFlying = 600.f;
	minSprintSpeedWhenFlying = 300.f;
	lastCameraFOV = 0.f;
	lastSpringArmTargetArmLength = 0.f;

	bIsSprinting = false;
	bIsMovingWithKeyboard = false;
	bCanBePossessed = true;
	bCanPossesByInputAction = false;

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
}

void ASimpleCharacter::Restart()
{
	Super::Restart();
	GetController()->SetControlRotation(lastControlRotation);

	GetRootComponent()->SetVisibility(true, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ASimpleCharacter::UnPossessed()
{
	// to do: check why is this unPossessed triggering when start playing
	Super::UnPossessed();
}

void ASimpleCharacter::BeginPlay()
{
	Super::BeginPlay();

	maxWalkSpeedMain = GetCharacterMovement()->MaxWalkSpeed;
	maxFlySpeedMain = GetCharacterMovement()->MaxFlySpeed;

	updateFunctionFloat.BindDynamic(this, &ASimpleCharacter::CamTransitionOnPossessionProgress);
	if (possessionCamTransitionFloatCurve) possessionCamTransitionTimeline->AddInterpFloat(possessionCamTransitionFloatCurve, updateFunctionFloat);
	
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

void ASimpleCharacter::Destroy()
{
	Super::Destroy();
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Actor Destroyed"));
}


// tricky coversion from world to local location.
void ASimpleCharacter::setLastSpringArmLocation(FVector worldLocation)
{
	FVector ogLocation = springArm->GetComponentLocation();
	springArm->SetWorldLocation(worldLocation);
	lastSpringArmLocation = springArm->GetRelativeLocation();
	springArm->SetWorldLocation(ogLocation);
}

void ASimpleCharacter::setLastValues(float SATargetArmLength, float cameraFieldOfView, FVector SASocketOffset, FVector SALocation, FRotator controlRotation)
{
	lastSpringArmTargetArmLength = SATargetArmLength; // to do: IMPORTANT: remove this variable and replace it with the lastSASocketOffset
	lastCameraFOV = cameraFieldOfView;
	lastSASocketOffset = SASocketOffset;
	setLastSpringArmLocation(SALocation);
	lastControlRotation = controlRotation;
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
	if (!bCanPossesByInputAction) return;
	
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

	/*Debug*/
	DrawDebugSphere(GetWorld(), hitResult.Location, 14.5f, 12, FColor::Blue, false, 2.f);
	DrawDebugLine(GetWorld(), traceStartLocation, traceEndLocation, hitResult.bBlockingHit ? FColor::Green : FColor::Red, false, 2.f);
	/*Debug*/

	if (!hitResult.bBlockingHit || !hitCharacter || !UCombatStatics::ApplyPossession(this, GetController(), hitCharacter)) return;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("Possession Succeded"));
	GetRootComponent()->SetVisibility(false, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (getPossessorPawn())
	{
		GetWorldTimerManager().ClearTimer(destroyActorTimerHandle);
		GetWorldTimerManager().SetTimer(destroyActorTimerHandle, this, &ASimpleCharacter::Destroy, 3.f, false);
	}
}

// to do: refactor/improve this code
void ASimpleCharacter::InputActionPossessionAbilityTriggered(const FInputActionInstance& Instance)
{
	if (!possessorPawn) return GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("¡There is not a possessorPawn Ref!"));
	
	ASimpleCharacter* possessor = Cast<ASimpleCharacter>(possessorPawn);
	if (!possessor) return GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("¡Invalid Cast to ASimpleCharacter!"));

	AController* controller = GetController();
	controller->UnPossess();
	GetRootComponent()->SetVisibility(false, true); // to do: we should move this to another place but, for now, it works here.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	float capsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	possessor->GetSpringArmComponent()->bEnableCameraLag = false;
	possessor->SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z+(capsuleHalfHeight-(capsuleHalfHeight*0.5))));

	possessor->setLastValues(GetSpringArmComponent()->TargetArmLength, GetCamera()->FieldOfView, GetSpringArmComponent()->SocketOffset, GetSpringArmComponent()->GetComponentLocation(), controller->GetControlRotation());

	controller->Possess(possessor);

	possessor->possessionCamTransitionTimeline->PlayFromStart();

	GetWorldTimerManager().ClearTimer(destroyActorTimerHandle);
	GetWorldTimerManager().SetTimer(destroyActorTimerHandle, this, &ASimpleCharacter::Destroy, 3.f, false);
}

void ASimpleCharacter::CamTransitionOnPossessionProgress(float value)
{
	FVector newSALocation = FMath::Lerp(lastSpringArmLocation, FVector(0), value); // to do: DON'T SET THE SPRING ARM LOCATION USE ONLY SOCKET OFFSET. BECAUSE IS ANNPOYING FO RHT PLAYER WHILE MOVING or fix it in some other way // note: dispossession is not doing this, only the possession
	FVector newSASocketOffset = FMath::Lerp(lastSASocketOffset, springArm->socketOffsetMain, value);
	float newSATargetArmLength = FMath::Lerp(lastSpringArmTargetArmLength, springArm->targetArmLengthMain, value);
	float newCamFieldOfView = FMath::Lerp(lastCameraFOV, camera->fieldOfViewMain, value);

	springArm->SetRelativeLocation(newSALocation);
	springArm->SocketOffset = newSASocketOffset;
	springArm->TargetArmLength = newSATargetArmLength;
	camera->SetFieldOfView(newCamFieldOfView);
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

APawn* ASimpleCharacter::getPossessorPawn()
{
	return possessorPawn;
}

float ASimpleCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); // to do: check if we should or shouldn't use this.
	if (!CanBeDamaged()) return 0.f;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, TEXT("I have been damaged by: ") + EventInstigator->GetActorNameOrLabel() + TEXT(" For: ") + FString::SanitizeFloat(DamageAmount) + TEXT(" Damage Points"));

	return DamageAmount;
}

bool ASimpleCharacter::TakePossession(APawn* ogPossessorPawn, AController* possessorController, USpringArmComponent* possessorSpringArmComp, UCameraComponent* possessorCameraComp)	
{
	if (!bCanBePossessed) return false;

	DrawDebugSphere(GetWorld(), GetSpringArmComponent()->GetComponentTransform().InverseTransformPosition(possessorCameraComp->GetComponentLocation()), 10.f, 25, FColor::Red);
	
	springArm->bEnableCameraLag = false;
	setLastValues(possessorSpringArmComp->TargetArmLength, possessorCameraComp->FieldOfView, possessorSpringArmComp->SocketOffset, possessorSpringArmComp->GetComponentLocation(), possessorController->GetControlRotation());

	possessorController->Possess(this);	
	this->possessorPawn = ogPossessorPawn;

	possessionCamTransitionTimeline->PlayFromStart();
	
	return true;
}
