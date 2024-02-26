// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCameraComponent.h"

UMainCameraComponent::UMainCameraComponent()
{
	fieldOfViewMain = FieldOfView;
	fieldOfViewAim = FieldOfView;
}

void UMainCameraComponent::BeginPlay()
{
	fieldOfViewMain = FieldOfView;
}
