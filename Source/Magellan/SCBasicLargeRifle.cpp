// Fill out your copyright notice in the Description page of Project Settings.

#include "SCBasicLargeRifle.h"

// Sets default values for this component's properties
USCBasicLargeRifle::USCBasicLargeRifle()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SpawnLocationComp = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocationComp"));
	SpawnLocationComp->SetupAttachment(this);
}


// Called when the game starts
void USCBasicLargeRifle::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Basic Large Rifle says Hello"));
}


// Called every frame
void USCBasicLargeRifle::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void USCBasicLargeRifle::StartFire()
{
	FVector Loc = GetComponentLocation();
	FRotator Rot = GetComponentRotation();
}

void USCBasicLargeRifle::EndFire()
{

}

void USCBasicLargeRifle::Fire()
{

}

