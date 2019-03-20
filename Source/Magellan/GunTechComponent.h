// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "GunTechComponent.generated.h"

class ABulletActor;

/**
 * 
 */
UCLASS()
class MAGELLAN_API UGunTechComponent : public UTechComponent
{
	GENERATED_BODY()
	
public:
	virtual void ActivateTechComponent() override;

protected:
	UFUNCTION()
	void Fire(TSubclassOf<AActor> Ammo);

};
