// Fill out your copyright notice in the Description page of Project Settings.

#include "BeamTechComponent.h"
#include "Kismet/GameplayStatics.h"

void UBeamTechComponent::ActivateTechComponent()
{
	Fire();
}

void UBeamTechComponent::Fire()
{
	if (MyParticles != nullptr)
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
		UGameplayStatics::SpawnEmitterAttached(MyParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
	}
}