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
	
}

void ATechActor::InitTechActor(AMechCharacter* TechOwner)
{
	if (TechOwner != nullptr)
	{
		MyMechCharacter = TechOwner;

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
					MyTechComponent->SetOwner(MyMechCharacter);
				}
			}
		}
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

	UpdateAimPoint();
}

void ATechActor::ActivateTech()
{
	MyTechComponent->ActivateTechComponent();
}

void ATechActor::UpdateArticulation(float DeltaTime)
{
	if (MyMechCharacter != nullptr)
	{
		FVector TargetVector = MyMechCharacter->GetLookVector() - GetActorLocation();
		FVector CurrentVector = GetActorForwardVector();
		FVector InterpVector = FMath::VInterpTo(CurrentVector, TargetVector, DeltaTime, ArticulationSpeed);
		SetActorRotation(InterpVector.Rotation());
	}
}

void ATechActor::UpdateAimPoint()
{
	bool HitResult = false;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));
	FHitResult Hit;
	TArray<AActor*> IgnoredActors;
	if (MyMechCharacter != nullptr)
	{
		IgnoredActors.Add(MyMechCharacter);
	}

	FVector RaycastVector = EmitPoint->GetForwardVector() * 50000.0f;
	FVector Start = EmitPoint->GetComponentLocation();
	FVector End = Start + RaycastVector;

	// Pew pew
	HitResult = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		TraceObjects,
		false,
		IgnoredActors,
		EDrawDebugTrace::None,
		Hit,
		true,
		FLinearColor::White, FLinearColor::Red, 0.1f);

	if (HitResult && (!Hit.Actor->ActorHasTag("Ammo")))
	{
		AimPoint = Hit.ImpactPoint;
	}
	else
	{
		AimPoint = GetActorLocation() + (GetActorForwardVector() * 20000.0f);
	}
}

