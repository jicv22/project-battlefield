// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "MainSpringArmComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBATTLEFIELD_API UMainSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float targetArmLengthMain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float targetArmLengthAim;
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	FVector socketOffsetMain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	FVector socketOffsetAim;

public:
	UMainSpringArmComponent();

protected:
	virtual void BeginPlay() override;
};
