// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TechComponent.generated.h"

class AMechCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGELLAN_API UTechComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTechComponent();

	UFUNCTION()
	void InitTechComponent(AMechCharacter* NewOwner, float Cap);

	UFUNCTION()
	AMechCharacter* GetCharacter() { return MyMechCharacter; };

	UFUNCTION()
	void SetParticles(UParticleSystem* Partis);

	UFUNCTION(BlueprintCallable)
	float GetCapacity() { return Capacity; }
	
	UFUNCTION(BlueprintCallable)
	virtual void ActivateTechComponent();

	UFUNCTION(BlueprintCallable)
	virtual void DeactivateTechComponent();
	
	UFUNCTION()
	float GetAmmoSpeed() { return AmmoSpeed; }

	UFUNCTION()
	void DeliverHitTo(AMechCharacter* Target, FVector Location);
	
	
	UPROPERTY(EditDefaultsOnly)
	float AmmoSpeed = 35000.0f;

	UPROPERTY(EditDefaultsOnly)
	float HitDamage = 1.0f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AmmoType;

	UPROPERTY(EditDefaultsOnly)
	bool bAutomatic = true;

	UPROPERTY(EditDefaultsOnly)
	float RateOfFire = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* EmitPoint;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShake> CameraShakeOnActivate;

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimerHandle AutoFireTimer;

	UFUNCTION()
	void ShakeCamera();

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter;

	UPROPERTY(BlueprintReadOnly)
	UParticleSystem* MyParticles;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* HitParticles;

	UPROPERTY(BlueprintReadOnly)
	float Capacity = 0.0f;

	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
