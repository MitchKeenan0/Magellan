// Fill out your copyright notice in the Description page of Project Settings.

#include "TechActor.h"

// Sets default values
ATechActor::ATechActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TechRoot"));
	RootComponent = TechRoot;

	TechMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TechMesh"));
	TechMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATechActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATechActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

