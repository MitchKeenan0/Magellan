// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TechComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGELLAN_API UTechComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTechComponent();

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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
