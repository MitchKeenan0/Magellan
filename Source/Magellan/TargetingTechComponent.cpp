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
	// Dot check for all mech characters
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMechCharacter::StaticClass(), Actors);
	int NumActors = Actors.Num();
	if (NumActors > 1)
	{
		for (int i = 0; i < NumActors; i++)
		{
			if (Actors.IsValidIndex(i))
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(Actors[i]);
				if ((HitMech != nullptr) && (HitMech != MyMechCharacter) && !HitMech->IsDead())
				{
					if (HitMech->GetTeam() != MyMechCharacter->GetTeam())
					{
						
						// Check angle to aim point
						FVector ToMech = (HitMech->GetActorLocation() - MyMechCharacter->GetActorLocation()).GetSafeNormal();
						FVector ToAim = MyMechCharacter->GetAimComponent()->GetForwardVector().GetSafeNormal();
						float DotToMech = FVector::DotProduct(ToAim, ToMech);
						if (DotToMech >= 0.9f)
						{
							// Avoid duplicates
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
}