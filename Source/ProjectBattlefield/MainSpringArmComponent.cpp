// Fill out your copyright notice in the Description page of Project Settings.


#include "MainSpringArmComponent.h"

UMainSpringArmComponent::UMainSpringArmComponent()
{
	targetArmLengthMain = TargetArmLength;
	targetArmLengthAim = TargetArmLength;
	socketOffsetMain = SocketOffset;
	socketOffsetAim = SocketOffset;
	ProbeChannel = ECollisionChannel::ECC_Visibility;
}

void UMainSpringArmComponent::BeginPlay()
{
	Super::BeginPlay();
	targetArmLengthMain = TargetArmLength;
	socketOffsetMain = SocketOffset;
}
