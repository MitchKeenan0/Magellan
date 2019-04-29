// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingTechComponent.h"

void UTargetingTechComponent::ActivateTechComponent()
{
	StartFire();
}

void UTargetingTechComponent::DeactivateTechComponent()
{
	StopFire();
}

void UTargetingTechComponent::StartFire()
{
	// RaycastTimer
	GetWorld()->GetTimerManager().SetTimer(RaycastTimer, this, &UTargetingTechComponent::RaycastForHit, RaycastRate, true, 0.001f);

	// EndTimer
	GetWorld()->GetTimerManager().SetTimer(TraceEndTimer, this, &UTargetingTechComponent::StopFire, 1.0f, false, 1.0f);
}

void UTargetingTechComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(RaycastTimer);
}

void UTargetingTechComponent::RaycastForHit()
{
	FHitResult Hit;
	FVector LineStart = EmitPoint->GetComponentLocation() + (EmitPoint->GetForwardVector() * 500.0f);
	FVector LineEnd = LineStart + (EmitPoint->GetForwardVector() * RaycastRange);
	
	bool Linecast = GetWorld()->LineTraceSingleByChannel(
		Hit,
		LineStart,
		LineEnd,
		ECollisionChannel::ECC_Pawn);
	
	if (Linecast)
	{
		AMechCharacter* HitMech = Cast<AMechCharacter>(Hit.GetActor());
		if (HitMech != nullptr)
		{
			bool bAdd = true;
			
			int NumTargets = LockedTargets.Num();
			if (NumTargets > 0)
			{
				for (int i = 0; i < NumTargets; i++)
				{
					AActor* Test = LockedTargets[i];
					if ((Test != nullptr) && (Test == HitMech))
					{
						bAdd = false;
					}
				}
			}

			if (bAdd && 
				(LockedTargets.Num() < MaxTargets))
			{
				LockedTargets.Add(HitMech);
				
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Added Target"));
			}
		}

		//if (MyImpactParticles != nullptr)
		//{
		//	// Visuals
		//	FVector Location = Hit.ImpactPoint;
		//	FRotator Rotation = GetOwner()->GetActorForwardVector().Rotation();
		//	ImpactParticles = UGameplayStatics::SpawnEmitterAttached(MyImpactParticles, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
		//}
	}
}