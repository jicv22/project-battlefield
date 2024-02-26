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
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 loadedAmmo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 totalAmmoCurrently;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammo")
	int32 magazineSize;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ammunation")
	TSubclassOf<ABullet> bullet;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	FVector cannionLocation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float gunReloadCooldownTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float gunFireCooldownTime;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gun")
	float maxFiringDistance;
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

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	virtual void StartReloading();
	UFUNCTION()
	virtual void StartShooting();
	UFUNCTION()
	virtual void StopShooting();

protected:
	UFUNCTION()
	virtual void DeactivateGunFireCooldown();
	UFUNCTION()
	virtual void DeactivateReloadCooldown();
	UFUNCTION()
	virtual void Fire();
	UFUNCTION()
	virtual void Reload();

};
