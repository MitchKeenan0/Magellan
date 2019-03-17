// Fill out your copyright notice in the Description page of Project Settings.

#include "BeamTechComponent.h"
#include "Kismet/GameplayStatics.h"

void UBeamTechComponent::ActivateTechComponent()
{
	StartFire();
}

void UBeamTechComponent::DeactivateTechComponent()
{
	StopFire();
}

void UBeamTechComponent::StartFire()
{
	if (MyParticles != nullptr)
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
		ActiveParticles = UGameplayStatics::SpawnEmitterAttached(MyParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
	}
}

void UBeamTechComponent::StopFire()
{
	if (ActiveParticles != nullptr)
	{
		ActiveParticles->DestroyComponent();
		ActiveParticles = nullptr;
	}
}