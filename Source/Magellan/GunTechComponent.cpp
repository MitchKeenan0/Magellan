// Fill out your copyright notice in the Description page of Project Settings.

#include "GunTechComponent.h"
#include "BulletActor.h"

void UGunTechComponent::ActivateTechComponent()
{
	if ((Capacity > 0.0f) && (AmmoType != nullptr))
	{
		float TimeBetweenShots = (1.0f / RateOfFire);

		float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UGunTechComponent::Fire, TimeBetweenShots, true, FirstDelay);
		//GetWorld()->GetTimerManager().SetTimer(AimPointTimer, this, &ATechActor::UpdateAimPoint, 0.03f, true);
	}
}

void UGunTechComponent::DeactivateTechComponent()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
	
	RunningShotCount = 0;
}

void UGunTechComponent::Fire()
{
	if (Capacity > 0.0f)
	{
		FActorSpawnParameters SpawnInfo;
		FRotator ShotRotation = EmitPoint->GetForwardVector().Rotation();

		// Inaccuracy
		float TimeSinceLastFire = GetWorld()->TimeSeconds - LastFireTime;
		float ManualFireRate = (1.0f / RateOfFire) * 1.1f;
		if ((RunningShotCount >= ShotsBeforeSpread)
			|| (TimeSinceLastFire <= ManualFireRate))
		{
			float CompoundingError = FMath::Clamp(RunningShotCount * AccuracySpread, 1.0f, MaxSpread);
			FRotator SimpleMiscalculation = FMath::VRand().Rotation() * (0.01f * CompoundingError);
			ShotRotation += SimpleMiscalculation;
		} 
		
		RunningShotCount += 1;

		// Spawning
		AActor* NewBullet = GetWorld()->SpawnActor<AActor>(AmmoType, 
			EmitPoint->GetComponentLocation(), 
			ShotRotation, SpawnInfo);
		if (NewBullet != nullptr)
		{
			if (MyMechCharacter != nullptr)
			{
				ABulletActor* Bullet = Cast<ABulletActor>(NewBullet);
				if (Bullet != nullptr)
				{
					Bullet->InitBullet(this);
					Capacity -= 1.0f;

					if (!MyMechCharacter->IsBot())
					{
						ShakeCamera();
					}
				}

				LastFireTime = GetWorld()->TimeSeconds;
			}
		}
	}
	else /// Out of ammo
	{
		DeactivateTechComponent();
	}
}

