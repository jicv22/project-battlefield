// Fill out your copyright notice in the Description page of Project Settings.


#include "GunComponent.h"
#include "SimpleCharacter.h"
#include "OffensiveCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "CombatStatics.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	cannionLocation = FVector(0, 0, 0);
	gunReloadCooldownTime = 1.f;
	gunFireCooldownTime = .2f;
	maxFiringDistance = 20000.f;
	gunFireDisperssion = 1.f;
	maxGunRecoil = FRotator(-0.05, 0.05, 0);
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

int32 UGunComponent::getLoadedAmmo()
{
	return loadedAmmo;
}

int32 UGunComponent::getTotalAmmoCurrently()
{
	return totalAmmoCurrently;
}

int32 UGunComponent::getMagazineSize()
{
	return magazineSize;
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
			FVector cameraLocation = character->GetCameraComponent()->GetComponentLocation();
			FVector forwardVector = UKismetMathLibrary::GetForwardVector(character->GetControlRotation());
			FVector traceStartLocation = cameraLocation + (forwardVector * character->GetSpringArmComponent()->TargetArmLength);
			FVector traceEndLocation = traceStartLocation + (forwardVector * maxFiringDistance);

			FHitResult hitResult;
			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActor(character);
			GetWorld()->LineTraceSingleByChannel(hitResult, traceStartLocation, traceEndLocation, ECollisionChannel::ECC_WorldDynamic, queryParams);

			cannionLocation = character->GetActorLocation(); // to do: this cannion position will be the socket located in the robot's gun.
			FActorSpawnParameters spawnParameters;
			spawnParameters.Instigator = character;
			spawnParameters.Owner = character;
			FVector finalLookAtTarget = (hitResult.bBlockingHit ? hitResult.Location : traceEndLocation)+FVector(FMath::FRandRange(-gunFireDisperssion, gunFireDisperssion), FMath::FRandRange(-gunFireDisperssion, gunFireDisperssion), FMath::FRandRange(-gunFireDisperssion, gunFireDisperssion));
			FRotator actorRotation = UKismetMathLibrary::FindLookAtRotation(cannionLocation, finalLookAtTarget);

			GetWorld()->SpawnActor<ABullet>(bullet, cannionLocation, actorRotation, spawnParameters);

			double pitch = FMath::FRandRange(-maxGunRecoil.Pitch, maxGunRecoil.Pitch);
			double yaw = FMath::FRandRange(-maxGunRecoil.Yaw, maxGunRecoil.Yaw);
			double roll = FMath::FRandRange(-maxGunRecoil.Roll, maxGunRecoil.Roll);
			UCombatStatics::ApplyRecoil(FRotator(pitch,yaw,roll), Cast<ASimpleCharacter>(GetOwner()));

			// Debug
			if (hitResult.bBlockingHit)
			{
				DrawDebugLine(GetWorld(), traceStartLocation, hitResult.Location, FColor::Red, false, 5.f);
				DrawDebugLine(GetWorld(), hitResult.Location, traceEndLocation, FColor::Green, false, 5.f);
				DrawDebugSphere(GetWorld(), hitResult.Location, 5.f, 12, FColor::Green, false, 5.f);
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
