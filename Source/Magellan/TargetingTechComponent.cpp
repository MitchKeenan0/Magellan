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
	GetWorld()->GetTimerManager().SetTimer(TraceEndTimer, this, &UTargetingTechComponent::StopFire, 5.0f, false, 5.0f);

	// Update Timer
	if (!GetWorld()->GetTimerManager().IsTimerActive(UpdateTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &UTargetingTechComponent::UpdateTargets, 0.6f, true, 0.6f);
	}
}

void UTargetingTechComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UTargetingTechComponent::UpdateTargets()
{
	// Sift thru and remove targets
	int NumTargets = LockedTargets.Num();
	if (NumTargets > 0)
	{
		for (int i = 0; i < NumTargets; i++) /// cant use auto because we may alter this array here
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
	FVector LineStart = EmitPoint->GetComponentLocation();
	FVector LineEnd = LineStart + (EmitPoint->GetForwardVector() * RaycastRange);
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(300.0f, 500.0f, 500.0f));

	bool bHitBocks = GetWorld()->SweepMultiByChannel(Hits, LineStart, LineEnd, FQuat::Identity, ECollisionChannel::ECC_Visibility, Shape);
	if (bHitBocks)
	{
		int NumHits = Hits.Num();
		if (NumHits > 0)
		{
			for (int i = 0; i < NumHits; i++)
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(Hits[i].GetActor());
				if ((HitMech != nullptr) && (HitMech != MyMechCharacter))
				{
					if (HitMech->GetTeam() != MyMechCharacter->GetTeam())
					{
						// Decide whether to add target
						bool bAdd = true;
						int NumTargets = LockedTargets.Num();
						if (NumTargets > 0)
						{
							// Check for duplicate
							for (auto& Attotor : LockedTargets)
							{
								if (HitMech == Attotor)
								{
									bAdd = false;
								}
							}
						}

						// Target aquired
						if (bAdd &&
							(NumTargets <= MaxTargets))
						{
							// Clear the array
							LockedTargets.Empty();

							if ((NumTargets >= 1) && LockedTargets.IsValidIndex(i))
							{
								LockedTargets.RemoveSingle(LockedTargets[i]);
							}

							LockedTargets.Add(HitMech);

							HitMech->ReceiveLock();
						}
					}
					
				}
			}
		}
	}
}