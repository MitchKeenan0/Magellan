// Fill out your copyright notice in the Description page of Project Settings.

#include "TechActor.h"
#include "MechCharacter.h"

// Sets default values
ATechActor::ATechActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TechRoot"));
	RootComponent = TechRoot;

	TechMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TechMesh"));
	TechMesh->SetupAttachment(RootComponent);

	EmitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("EmitPoint"));
	EmitPoint->SetupAttachment(RootComponent);

	//MyTechComponent = CreateDefaultSubobject<UTechComponent>(TEXT("MyTechComponent"));
	
}

// Called when the game starts or when spawned
void ATechActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize Tech Component
	if (TechComponentSubclass != nullptr)
	{
		MyTechComponent = NewObject<UTechComponent>(this, *TechComponentSubclass);
		if (MyTechComponent != nullptr)
		{
			MyTechComponent->RegisterComponent();
			if (AmmoType != nullptr)
			{
				MyTechComponent->AmmoType = AmmoType;
				MyTechComponent->EmitPoint = EmitPoint;
			}
		}
	}
}

void ATechActor::InitTechActor(AMechCharacter* TechOwner)
{
	if (TechOwner != nullptr)
	{
		MyMechCharacter = TechOwner;
	}
}

// Called every frame
void ATechActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bArticulated)
	{
		UpdateArticulation(DeltaTime);
	}
}

void ATechActor::ActivateTech()
{
	MyTechComponent->ActivateTechComponent();
}

void ATechActor::UpdateArticulation(float DeltaTime)
{
	if (MyMechCharacter != nullptr)
	{
		FVector TargetVector = MyMechCharacter->GetLookVector();
		FVector CurrentVector = GetActorForwardVector();

		FVector InterpVector = FMath::VInterpConstantTo(CurrentVector, TargetVector, DeltaTime, ArticulationSpeed);
		SetActorRotation(InterpVector.Rotation());

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("Articulating..."));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("MyMechCharacter is nullptr"));
	}
}

