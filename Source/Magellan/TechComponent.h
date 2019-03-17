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
	void SetOwner(AMechCharacter* NewOwner);

	UFUNCTION()
	void SetParticles(UParticleSystem* Partis);

	UFUNCTION(BlueprintCallable)
	virtual void ActivateTechComponent();

	UFUNCTION(BlueprintCallable)
	virtual void DeactivateTechComponent();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AmmoType;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* EmitPoint;

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter;

	UPROPERTY(BlueprintReadOnly)
	UParticleSystem* MyParticles;

	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
