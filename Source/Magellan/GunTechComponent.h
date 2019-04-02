// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "GunTechComponent.generated.h"

class ABulletActor;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MAGELLAN_API UGunTechComponent : public UTechComponent
{
	GENERATED_BODY()
	
public:
	virtual void ActivateTechComponent() override;
	virtual void DeactivateTechComponent() override;

protected:
	UFUNCTION()
	void Fire();

};
