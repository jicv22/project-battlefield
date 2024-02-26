// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Bullet.generated.h"

UCLASS()
class PROJECTBATTLEFIELD_API ABullet : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UDamageType> damageType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	float damage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* staticMeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UCapsuleComponent* capsuleComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UProjectileMovementComponent* projectileMovementComponent;

	UPROPERTY()
	FTimerHandle bulletDestructionTimerHandle;

public:	
	ABullet();

	UFUNCTION()
	UProjectileMovementComponent* GetProjetileMovementComponent();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void StartDestroyProcess(AActor* OtherActor);
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	void destroyBullet();

};
