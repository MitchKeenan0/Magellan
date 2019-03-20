// Fill out your copyright notice in the Description page of Project Settings.

#include "GunTechComponent.h"
#include "BulletActor.h"

void UGunTechComponent::ActivateTechComponent()
{
	if (AmmoType != nullptr)
	{
		Fire(AmmoType);
	}
}

void UGunTechComponent::Fire(TSubclassOf<AActor> Ammo)
{
	FActorSpawnParameters SpawnInfo;
	AActor* NewBullet = GetWorld()->SpawnActor<AActor>(Ammo, EmitPoint->GetComponentLocation(), EmitPoint->GetComponentRotation(), SpawnInfo);
	if (NewBullet != nullptr)
	{
		if (MyMechCharacter != nullptr)
		{
			ABulletActor* Bullet = Cast<ABulletActor>(NewBullet);
			if (Bullet != nullptr)
			{
				Bullet->InitBullet(this);
				Capacity -= 1.0f;
			}
		}
	}
}