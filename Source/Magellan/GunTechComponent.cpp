// Fill out your copyright notice in the Description page of Project Settings.

#include "GunTechComponent.h"
#include "BulletActor.h"

void UGunTechComponent::ActivateTechComponent()
{
	if ((Capacity > 0.0f) && (AmmoType != nullptr))
	{
		float TimeBetweenShots = (1.0f / RateOfFire);
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UGunTechComponent::Fire, TimeBetweenShots, true, 0.0f);
		//GetWorld()->GetTimerManager().SetTimer(AimPointTimer, this, &ATechActor::UpdateAimPoint, 0.03f, true);
	}
}

void UGunTechComponent::DeactivateTechComponent()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void UGunTechComponent::Fire()
{
	if (Capacity > 0.0f)
	{
		FActorSpawnParameters SpawnInfo;
		AActor* NewBullet = GetWorld()->SpawnActor<AActor>(AmmoType, EmitPoint->GetComponentLocation(), EmitPoint->GetComponentRotation(), SpawnInfo);
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
			}
		}
	}
	else {
		DeactivateTechComponent();
	}
}

