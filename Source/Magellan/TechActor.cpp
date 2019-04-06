// Fill out your copyright notice in the Description page of Project Settings.

#include "TechActor.h"
#include "MechCharacter.h"

// Sets default values
ATechActor::ATechActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TechRoot"));
	SetRootComponent(TechRoot);

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
				
				MyTechComponent->SetParticles(TechParticles);
				MyTechComponent->InitTechComponent(MyMechCharacter, Capacity);
				MyTechComponent->EmitPoint = EmitPoint;

				if (AmmoType != nullptr)
				{
					MyTechComponent->AmmoType = AmmoType;
				}

				// Set timer for aim point update
				//GetWorld()->GetTimerManager().SetTimer(AimPointTimer, this, &ATechActor::UpdateAimPoint, 0.01f, true, 0.0f);
			}
		}
	}
}

// Called every frame
void ATechActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyMechCharacter != nullptr)
	{
		if (bArticulated)
		{
			UpdateArticulation(DeltaTime);
		}

		UpdateAimPoint();
	}
}

void ATechActor::ActivateTech()
{
	if (MyTechComponent != nullptr)
	{
		if (MyTechComponent->GetCapacity() != 0.0f) /// -1 used by beams
		{
			APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (MyController != nullptr)
			{
				MyTechComponent->ActivateTechComponent();
				
				if (!MyMechCharacter->IsBot() && (CameraShakeOnActivate != nullptr))
				{
					MyController->ClientPlayCameraShake(CameraShakeOnActivate, 1.0f, ECameraAnimPlaySpace::CameraLocal, GetActorRotation());
				}
			}
		}
	}
}

void ATechActor::DeactivateTech()
{
	if (MyTechComponent != nullptr)
	{
		if (MyTechComponent->GetCapacity() != 0.0f) /// -1 used by beams
		{
			APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (MyController != nullptr)
			{
				MyTechComponent->DeactivateTechComponent();


				if (CameraShakeOnActivate != nullptr)
				{
					MyController->ClientStopCameraShake(CameraShakeOnActivate, false);
				}
			}

		}
	}
}

void ATechActor::UpdateArticulation(float DeltaTime)
{
	if (MyMechCharacter != nullptr)
	{
		FVector TargetVector = (MyMechCharacter->GetLookVector() - GetActorLocation());
		FRotator TargetRotation = TargetVector.Rotation();

		// Clamping relative to Torso
		if (MyMechCharacter->GetTorso() != nullptr)
		{
			FRotator MechTorsoRotation = MyMechCharacter->GetTorso()->GetComponentRotation();
			
			float tp = FMath::Abs(GetActorRotation().Pitch);
			float ty = FMath::Abs(GetActorRotation().Yaw);

			float mp = FMath::Abs(MechTorsoRotation.Pitch) + ArticulationPitch;
			float my = FMath::Abs(MechTorsoRotation.Yaw) + ArticulationYaw;

			if (tp > mp)
			{
				TargetRotation.Pitch = MechTorsoRotation.Pitch;
			}

			if ((ty > my) || (ArticulationYaw == 0.0f))
			{
				TargetRotation.Yaw = MechTorsoRotation.Yaw;
			}
		}

		// Interp to
		FRotator CurrentRotation = GetActorForwardVector().Rotation();
		FRotator InterpRotator;
		if (bSmoothArticulation)
		{
			InterpRotator = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, ArticulationSpeed);
		}
		else
		{
			InterpRotator = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, ArticulationSpeed);
		}

		SetActorRotation(InterpRotator);
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
		FLinearColor::White, FLinearColor::Red, 0.001f);

	if (HitResult && (!Hit.Actor->ActorHasTag("Ammo")))
	{
		AimPoint = Hit.ImpactPoint;
	}
	else
	{
		//FVector LocalOffset = MyMechCharacter->GetActorLocation();
		AimPoint = MyMechCharacter->GetLookVector();
	}
}

bool ATechActor::IsEquipped()
{
	bool Result = false;
	if (MyMechCharacter != nullptr)
	{
		Result = true;
	}
	return Result;
}

float ATechActor::GetCapacity()
{
	float Result = 0.0f;

	if (MyTechComponent != nullptr)
	{
		Result = MyTechComponent->GetCapacity();
	}

	return Result;
}
