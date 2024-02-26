// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "MainCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBATTLEFIELD_API UMainCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "CameraSettings")
	float fieldOfViewMain;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraSettings")
	float fieldOfViewAim;

public:
	UMainCameraComponent();

protected:
	virtual void BeginPlay() override;
};
