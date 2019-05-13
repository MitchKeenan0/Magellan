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
	TArray<FHitResult> Hits;
	FVector LineStart = EmitPoint->GetComponentLocation() + (EmitPoint->GetForwardVector() * 300.0f);
	FVector LineEnd = LineStart + (EmitPoint->GetForwardVector() * RaycastRange);
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(10.0f, 250.0f, 10.0f));

	bool bHitBocks = GetWorld()->SweepMultiByChannel(Hits, LineStart, LineEnd, FQuat::Identity, ECollisionChannel::ECC_Visibility, Shape);
	if (bHitBocks)
	{
		int NumHits = Hits.Num();
		if (NumHits > 0)
		{
			for (int i = 0; i < NumHits; i++)
			{
				if (Hits[i].GetActor() != MyMechCharacter)
				{
					// Formal hit & damage
					AMechCharacter* HitMech = Cast<AMechCharacter>(Hits[i].GetActor());
					if ((HitMech != nullptr) && (!HitMech->IsDead()))
					{
						DeliverHitTo(HitMech, Hits[i].ImpactPoint);

						TSubclassOf<UDamageType> DmgType;
						UGameplayStatics::ApplyDamage(HitMech, HitDamage, MyMechCharacter->GetController(), MyMechCharacter, DmgType);

						MyMechCharacter->ConfirmHit();
					}

					if (MyImpactParticles != nullptr)
					{
						// Visuals
						FVector Location = Hits[i].ImpactPoint;
						FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
						ImpactParticles = UGameplayStatics::SpawnEmitterAttached(MyImpactParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
					}
				}
			}
		}
	}
}