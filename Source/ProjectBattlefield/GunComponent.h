// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet.h"
#include "Components/ActorComponent.h"
#include "GunComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTBATTLEFIELD_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ammunation")
	TSubclassOf<ABullet> bullet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	FVector cannionLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	FRotator maxGunRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 loadedAmmo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 totalAmmoCurrently;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 magazineSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float gunReloadCooldownTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float gunFireCooldownTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float maxFiringDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float gunFireDisperssion;

	UPROPERTY(BlueprintReadWrite, Category = "Gun")
	bool bIsReloadCooldownActive;
	UPROPERTY(BlueprintReadWrite, Category = "Gun")
	bool bIsGunFireCooldownActive;

	UPROPERTY(BlueprintReadWrite, Category = "Gun Timers")
	FTimerHandle gunFireCooldownTimerHandle;
	UPROPERTY(BlueprintReadWrite, Category = "Gun Timers")
	FTimerHandle gunFireCooldownDeactivatorTimerHandle;
	UPROPERTY(BlueprintReadWrite, Category = "Gun Timers")
	FTimerHandle gunReloadCooldownDeactivatorTimerHandle;

public:
	UGunComponent();

	int32 getLoadedAmmo();
	int32 getTotalAmmoCurrently();
	int32 getMagazineSize();

	UFUNCTION()
	virtual void StartReloading();
	UFUNCTION()
	virtual void StartShooting();
	UFUNCTION()
	virtual void StopShooting();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void DeactivateGunFireCooldown();
	UFUNCTION()
	virtual void DeactivateReloadCooldown();
	UFUNCTION()
	virtual void Fire();
	UFUNCTION()
	virtual void Reload();

};
