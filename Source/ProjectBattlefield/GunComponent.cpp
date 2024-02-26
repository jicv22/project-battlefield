// Fill out your copyright notice in the Description page of Project Settings.


#include "GunComponent.h"
#include "SimpleCharacter.h"
#include "OffensiveCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	cannionLocation = FVector(0, 0, 0);
	gunReloadCooldownTime = 1.f;
	gunFireCooldownTime = .2f;
	maxFiringDistance = 20000.f;
	totalAmmoCurrently = 100;
	loadedAmmo = 10;
	magazineSize = 10;
	bIsReloadCooldownActive = false;
	bIsGunFireCooldownActive = false;
}

void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UGunComponent::StartReloading()
{
	if (!bIsReloadCooldownActive && loadedAmmo < magazineSize && totalAmmoCurrently > 0)
	{
		bIsReloadCooldownActive = true;
		GetWorld()->GetTimerManager().ClearTimer(gunReloadCooldownDeactivatorTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(gunReloadCooldownDeactivatorTimerHandle, this, &UGunComponent::Reload, gunReloadCooldownTime, false);
		GEngine->AddOnScreenDebugMessage(-1, gunReloadCooldownTime, FColor::Red, TEXT("Reloading"));
	}
}

void UGunComponent::StartShooting()
{	
	GetWorld()->GetTimerManager().ClearTimer(gunFireCooldownTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(gunFireCooldownTimerHandle, this, &UGunComponent::Fire, gunFireCooldownTime, true, 0);
}

void UGunComponent::StopShooting()
{
	GetWorld()->GetTimerManager().ClearTimer(gunFireCooldownTimerHandle);
}

void UGunComponent::DeactivateGunFireCooldown()
{
	bIsGunFireCooldownActive = false;
	GetWorld()->GetTimerManager().ClearTimer(gunFireCooldownDeactivatorTimerHandle);
}

void UGunComponent::DeactivateReloadCooldown()
{
	bIsReloadCooldownActive = false;
	GetWorld()->GetTimerManager().ClearTimer(gunReloadCooldownDeactivatorTimerHandle);
}

void UGunComponent::Fire()  // to do: Try to do this more standard.
{
	// to do: improve/refactor all this code.
	if (!bIsReloadCooldownActive)
	{
		if (loadedAmmo > 0 && !bIsGunFireCooldownActive)
		{
			bIsGunFireCooldownActive = true;
			loadedAmmo--;
			GetWorld()->GetTimerManager().ClearTimer(gunFireCooldownDeactivatorTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(gunFireCooldownDeactivatorTimerHandle, this, &UGunComponent::DeactivateGunFireCooldown, gunFireCooldownTime - 0.05f, false);

			ASimpleCharacter* character = Cast<ASimpleCharacter>(GetOwner());
			FVector cameraLocation = character->camera->GetComponentLocation();
			FVector forwardVector = UKismetMathLibrary::GetForwardVector(character->GetControlRotation());
			FVector traceStartLocation = cameraLocation + (forwardVector * character->springArm->TargetArmLength);
			FVector traceEndLocation = traceStartLocation + (forwardVector * maxFiringDistance);

			FHitResult middleHitResult;
			FHitResult lowerHitResult;
			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActor(character);
			GetWorld()->LineTraceSingleByChannel(middleHitResult, traceStartLocation, traceEndLocation, ECollisionChannel::ECC_WorldDynamic, queryParams);

			cannionLocation = character->GetActorLocation(); // to do: this cannion position will be the socket located in the robot's gun.
			FActorSpawnParameters spawnParameters;
			spawnParameters.Instigator = character;
			spawnParameters.Owner = character;
			FRotator actorRotation = UKismetMathLibrary::FindLookAtRotation(cannionLocation, middleHitResult.bBlockingHit ? middleHitResult.Location : traceEndLocation);

			GetWorld()->SpawnActor<ABullet>(bullet, cannionLocation, actorRotation, spawnParameters);

			// Debug
			if (middleHitResult.bBlockingHit)
			{
				DrawDebugLine(GetWorld(), traceStartLocation, middleHitResult.Location, FColor::Red, false, 5.f);
				DrawDebugLine(GetWorld(), middleHitResult.Location, traceEndLocation, FColor::Green, false, 5.f);
				DrawDebugSphere(GetWorld(), middleHitResult.Location, 10.f, 25, FColor::Green, false, 5.f);
			}
			else
			{
				DrawDebugLine(GetWorld(), traceStartLocation, traceEndLocation, FColor::Red, false, 5.f);
			}
			// Debug
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Click (No Ammo)"));
		}
	}
}

void UGunComponent::Reload()
{
	DeactivateReloadCooldown();
	GetWorld()->GetTimerManager().ClearTimer(gunReloadCooldownDeactivatorTimerHandle);
	int newAmmoAmountToReload = 0;
	if (totalAmmoCurrently >= magazineSize)
	{
		newAmmoAmountToReload = magazineSize - loadedAmmo;
	}
	else
	{
		newAmmoAmountToReload = (magazineSize - loadedAmmo);
		if(newAmmoAmountToReload > totalAmmoCurrently)
		{
			newAmmoAmountToReload = totalAmmoCurrently;
		}
	}
	loadedAmmo += newAmmoAmountToReload;
	totalAmmoCurrently -= newAmmoAmountToReload;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Reloaded"));
}
