// Fill out your copyright notice in the Description page of Project Settings.

#include "MechOutfitComponent.h"

// Sets default values for this component's properties
UMechOutfitComponent::UMechOutfitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMechOutfitComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMechOutfitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UMechOutfitComponent::ClearOutfit()
{
	int TechNum = HardpointTechs.Num();
	if (TechNum > 0)
	{
		for (ATechActor* Tech : HardpointTechs)
		{
			if (Tech != nullptr)
			{
				Tech->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				Tech->SetLifeSpan(1.0f);
			}
		}

		//HardpointTechs.Empty();
	}
}

