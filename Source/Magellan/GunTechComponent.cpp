// Fill out your copyright notice in the Description page of Project Settings.

#include "GunTechComponent.h"

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
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Gun aquired mech character"));
		}
	}
}