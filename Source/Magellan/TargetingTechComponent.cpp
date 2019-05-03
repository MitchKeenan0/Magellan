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

	// Update Timer
	if (!GetWorld()->GetTimerManager().IsTimerActive(UpdateTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &UTargetingTechComponent::UpdateTargets, 0.2f, true, 0.2f);
	}
}

void UTargetingTechComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(RaycastTimer);
}

void UTargetingTechComponent::UpdateTargets()
{
	// Sift thru and remove targets
	int NumTargets = LockedTargets.Num();
	if (NumTargets > 0)
	{
		
		for (int i = 0; i < NumTargets; i++)
		{
			if (LockedTargets[i] != nullptr)
			{
				AMechCharacter* MechTarget = Cast<AMechCharacter>(LockedTargets[i]);
				if (MechTarget != nullptr)
				{

					if (MechTarget->IsDead())
					{
						LockedTargets.RemoveSingleSwap(LockedTargets[i]);
					}
				}
			}
		}
	}
}

void UTargetingTechComponent::RaycastForHit()
{
	TArray<FHitResult> Hits;
	FVector LineStart = EmitPoint->GetComponentLocation() + (EmitPoint->GetForwardVector() * 500.0f);
	FVector LineEnd = LineStart + (EmitPoint->GetForwardVector() * RaycastRange);
	
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(1250.0f, 550.0f, 550.0f));
	FHitResult SweepResult;
	const FQuat Rotation = EmitPoint->GetComponentRotation().Quaternion();
	bool Linecast = GetWorld()->SweepMultiByChannel(Hits, LineStart, LineEnd, Rotation, ECollisionChannel::ECC_Visibility, Shape);
		///(Hits, StartLocation, EndLocation, ShapeRotation, ECC_Visible, Shape);

		/*GetWorld()->LineTraceSingleByChannel(
		Hit,
		LineStart,
		LineEnd,
		ECollisionChannel::ECC_Pawn);*/
	
	if (Linecast)
	{
		int NumHits = Hits.Num();
		if (NumHits > 0)
		{
			for (int i = 0; i < NumHits; i++)
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(Hits[i].GetActor());
				if ((HitMech != nullptr) && (HitMech != MyMechCharacter))
				{
					bool bAdd = true;

					int NumTargets = LockedTargets.Num();
					if (NumTargets > 0)
					{
						for (int j = 0; j < NumTargets; j++)
						{
							// Check for duplicate
							AActor* Test = LockedTargets[j];
							if ((Test != nullptr) && (Test == HitMech))
							{
								bAdd = false;
							}

							// Check for team ID
							if (HitMech->GetTeam() == MyMechCharacter->GetTeam())
							{
								bAdd = false;
							}
						}
					}

					// Target aquired
					if (bAdd &&
						(LockedTargets.Num() < MaxTargets))
					{
						LockedTargets.Add(HitMech);

						HitMech->ReceiveLock();
					}
				}
			}
		}
	}
}