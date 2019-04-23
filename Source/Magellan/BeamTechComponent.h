// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "MechCharacter.h"
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
	UPROPERTY(EditDefaultsOnly)
	float RaycastRate = 0.01f;

	UPROPERTY(EditDefaultsOnly)
	float RaycastRange = 55000.0f;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MyImpactParticles;

	UPROPERTY()
	FTimerHandle RaycastTimer;

	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void RaycastForHit();

	UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* ActiveParticles;

	UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* ImpactParticles;
};
