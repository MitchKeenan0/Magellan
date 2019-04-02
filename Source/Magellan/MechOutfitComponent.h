// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Magellan.h"
#include "Components/ActorComponent.h"
#include "TechActor.h"
#include "MechOutfitComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGELLAN_API UMechOutfitComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMechOutfitComponent();

	UFUNCTION(BlueprintCallable)
	TArray<ATechActor*> GetHardpointTechs() { return HardpointTechs; }			/// CRASH HERE- somewhere at begin play HardpointTechs is null

	UFUNCTION()
	void ClearOutfit();

	UPROPERTY(EditDefaultsOnly)
	TArray<ATechActor*> ComputerTechs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ATechActor*> EngineTechs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<ATechActor*> HardpointTechs;

	UPROPERTY(EditDefaultsOnly)
	TArray<FVector> HardpointLocations;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/// tbd
	//UPROPERTY(EditDefaultsOnly)
	//TArray<ATechActor*> ComputerTechs;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
