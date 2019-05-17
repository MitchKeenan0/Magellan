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
	// Clear the array
	LockedTargets.Empty();

	// RaycastTimer
	GetWorld()->GetTimerManager().SetTimer(RaycastTimer, this, &UTargetingTechComponent::RaycastForHit, RaycastRate, true, 0.001f);

	// Update Timer
	GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &UTargetingTechComponent::UpdateTargets, 0.05f, true, 0.05f);

	// EndTimer
	//GetWorld()->GetTimerManager().SetTimer(TraceEndTimer, this, &UTargetingTechComponent::StopFire, 5.0f, false, 5.0f);
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
		for (int i = 0; i < NumTargets; i++) /// cant use auto because we may alter this array here
		{
			if (LockedTargets[i] != nullptr)
			{
				AMechCharacter* MechTarget = Cast<AMechCharacter>(LockedTargets[i]);
				if ((MechTarget != nullptr) && MechTarget->IsDead())
				{
					LockedTargets.RemoveSingleSwap(LockedTargets[i]);
					
					MyMechCharacter->OnTargetLockDelegate.Broadcast(false);
				}
			}
			else
			{
				MyMechCharacter->OnTargetLockDelegate.Broadcast(false);
			}
		}
	}
}

void UTargetingTechComponent::RaycastForHit()
{
	TArray<FHitResult> DirectHits;
	TArray<FHitResult> Hits;
	FVector LineStart = EmitPoint->GetComponentLocation() + (EmitPoint->GetForwardVector() * 500.0f);
	FVector LineEnd = EmitPoint->GetForwardVector() * RaycastRange;
	FQuat Rote = MyMechCharacter->GetAimPoint().Rotation().Quaternion();
	Rote.W = 45.0f;
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(500.0f, 1050.0f, 1050.0f));

	bool Test = GetWorld()->LineTraceMultiByChannel(DirectHits, LineStart, LineStart + LineEnd, ECollisionChannel::ECC_Visibility);
	bool bHitBocks = GetWorld()->SweepMultiByChannel(Hits, LineStart, LineEnd, Rote, ECollisionChannel::ECC_Visibility, Shape);
	
	if (Test)
	{
		int NumDirectHits = DirectHits.Num();
		if (NumDirectHits > 0)
		{
			for (int i = 0; i < NumDirectHits; i++)
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(DirectHits[i].GetActor());
				if ((HitMech != nullptr) && (HitMech != MyMechCharacter) && !HitMech->IsDead())
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
							(NumTargets < MaxTargets))
						{
							LockedTargets.Add(HitMech);

							HitMech->ReceiveLock();
						}
					}

				}
			}
		}
	}
	else if (bHitBocks)
	{
		int NumHits = Hits.Num();
		if (NumHits > 0)
		{
			for (int i = 0; i < NumHits; i++)
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(Hits[i].GetActor());
				if ((HitMech != nullptr) && (HitMech != MyMechCharacter) && !HitMech->IsDead())
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
							(NumTargets < MaxTargets))
						{
							LockedTargets.Add(HitMech);

							HitMech->ReceiveLock();
						}
					}
					
				}
			}
		}
	}
}