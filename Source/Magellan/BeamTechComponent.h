// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "BeamTechComponent.generated.h"

/**
 * 
 */
UCLASS()
class MAGELLAN_API UBeamTechComponent : public UTechComponent
{
	GENERATED_BODY()
	
public:
	virtual void ActivateTechComponent() override;

protected:
	UFUNCTION()
	void Fire();
};
