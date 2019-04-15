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

		// Accuracy rotation
		FRotator ShotRotation = EmitPoint->GetComponentRotation();
		if (RunningShotCount > ShotsBeforeSpread)
		{
			float CompoundingError = FMath::Clamp(RunningShotCount * AccuracySpread, 1.0f, MaxSpread);
			FRotator Inaccuracy = (FMath::VRand() * CompoundingError).Rotation() * 0.01f;
			ShotRotation += Inaccuracy;
		}

		AActor* NewBullet = GetWorld()->SpawnActor<AActor>(AmmoType, EmitPoint->GetComponentLocation(), ShotRotation, SpawnInfo);
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

				RunningShotCount += 1;
			}
		}
	}
	else {
		DeactivateTechComponent();
	}
}

