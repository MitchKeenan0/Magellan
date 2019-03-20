// Fill out your copyright notice in the Description page of Project Settings.

#include "TechComponent.h"
#include "MechCharacter.h"

// Sets default values for this component's properties
UTechComponent::UTechComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTechComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UTechComponent::InitTechComponent(AMechCharacter* NewOwner, float Cap)
{
	if (NewOwner != nullptr)
	{
		MyMechCharacter = NewOwner;
		Capacity = Cap;
	}

	// for later__ tie capacity to Output if (Cap == -1) 
}

void UTechComponent::SetParticles(UParticleSystem* Partis)
{
	if (Partis != nullptr)
	{
		MyParticles = Partis;
	}
}


// Called every frame
void UTechComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTechComponent::ActivateTechComponent()
{
	
}

void UTechComponent::DeactivateTechComponent()
{

}

