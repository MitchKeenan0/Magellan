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

void UTechComponent::ShakeCamera()
{
	APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if ((MyController != nullptr) && (MyMechCharacter != nullptr) && !MyMechCharacter->IsBot())
	{
		if (CameraShakeOnActivate != nullptr)
		{
			MyController->ClientPlayCameraShake(CameraShakeOnActivate, 1.0f, ECameraAnimPlaySpace::CameraLocal, MyMechCharacter->GetActorRotation());
		}
	}
}

void UTechComponent::DeliverHitTo(AMechCharacter* Target, FVector Location)
{
	if (HitParticles != nullptr)
	{
		UParticleSystem* DamageParticles = Cast<UParticleSystem>(UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, Location, FRotator::ZeroRotator, true));
		
		if (Target->GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			ShakeCamera();
		}
	}
}

