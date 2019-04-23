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
		// RaycastTimer
		GetWorld()->GetTimerManager().SetTimer(RaycastTimer, this, &UBeamTechComponent::RaycastForHit, RaycastRate, true, 0.001f);

		// Visuals
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
		ActiveParticles = UGameplayStatics::SpawnEmitterAttached(MyParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
		
		ShakeCamera();
	}
}

void UBeamTechComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(RaycastTimer);

	if (ActiveParticles != nullptr)
	{
		ActiveParticles->DestroyComponent();
		ActiveParticles = nullptr;
	}
}

void UBeamTechComponent::RaycastForHit()
{
	FHitResult Hit;
	FVector LineStart = EmitPoint->GetComponentLocation();
	FVector LineEnd = LineStart + (EmitPoint->GetForwardVector() * RaycastRange);
	bool Linecast = GetWorld()->LineTraceSingleByChannel(
		Hit,
		LineStart,
		LineEnd,
		ECollisionChannel::ECC_Pawn);
	if (Linecast)
	{
		// Formal hit & damage
		AMechCharacter* HitMech = Cast<AMechCharacter>(Hit.GetActor());
		if (HitMech != nullptr)
		{
			if (HitMech->GetLifeSpan() == 0.0f) /// hacky temp case for "not dead"
			{
				MyMechCharacter->ConfirmHit();

				if (HitMech->IsBot())
				{
					HitMech->DestructMech();
				}
			}
		}

		if (MyImpactParticles != nullptr)
		{
			// Visuals
			FVector Location = Hit.ImpactPoint;
			FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
			ImpactParticles = UGameplayStatics::SpawnEmitterAttached(MyImpactParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
		}
	}
}