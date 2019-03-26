// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "BeamTechComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MAGELLAN_API UBeamTechComponent : public UTechComponent
{
	GENERATED_BODY()
	
public:
	virtual void ActivateTechComponent() override;
	virtual void DeactivateTechComponent() override;

protected:
	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();

	UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* ActiveParticles;
};
