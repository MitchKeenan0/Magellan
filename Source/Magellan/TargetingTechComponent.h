// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TechComponent.h"
#include "MechCharacter.h"
#include "TargetingTechComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MAGELLAN_API UTargetingTechComponent : public UTechComponent
{
	GENERATED_BODY()
	

public:
	virtual void ActivateTechComponent() override;
	virtual void DeactivateTechComponent() override;

	UFUNCTION()
	TArray<AActor*> GetLockedTargets() { return LockedTargets; }

protected:
	UPROPERTY(EditDefaultsOnly)
	float RaycastRate = 0.01f;

	UPROPERTY(EditDefaultsOnly)
	float RaycastRange = 55000.0f;

	UPROPERTY(EditDefaultsOnly)
	int MaxTargets = 3.0f;

	UPROPERTY()
	FTimerHandle RaycastTimer;

	UPROPERTY()
	FTimerHandle TraceEndTimer;

	UPROPERTY()
	FTimerHandle UpdateTimer;

	UPROPERTY()
	TArray<AActor*> LockedTargets;


	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void RaycastForHit();

	UFUNCTION()
	void UpdateTargets();

	/*UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* ActiveParticles;*/

	/*UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* ImpactParticles;*/

	/*UPROPERTY(EditDefaultsOnly)
	UParticleSystem* MyImpactParticles;*/
};
