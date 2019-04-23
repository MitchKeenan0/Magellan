// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletActor.h"

// Sets default values
ABulletActor::ABulletActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABulletActor::OnBulletBeginOverlap);
	CollisionBox->OnComponentHit.AddDynamic(this, &ABulletActor::OnBulletHit);*/
	

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel1);
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SetRootComponent(MeshComp);
	
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ParticleComp->bAbsoluteScale = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	//ProjectileMovement->InitialSpeed = ProjectileSpeed;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));

	Tags.Add("Ammo");
}

// Called when the game starts or when spawned
void ABulletActor::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
void ABulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHit)
	{
		LineTraceForHit();
	}
}


void ABulletActor::InitBullet(UTechComponent* Shooter)
{
	if (Shooter != nullptr)
	{
		MyTechComponent = Shooter;
		MyMechCharacter = MyTechComponent->GetCharacter();

		// Inherit bullet velocity from moving shooter
		if (MyMechCharacter != nullptr)
		{
			FVector MechVelocity = MyMechCharacter->GetVelocity();
			FVector CurrentV = ProjectileMovement->Velocity;
			FVector NewV = CurrentV + MechVelocity;
			ProjectileMovement->Velocity += NewV;

			// Launch!
			GetWorld()->GetTimerManager().SetTimer(LaunchTimer, this, &ABulletActor::LaunchBullet, 0.05f, false, 0.05f);
		}
	}
}

void ABulletActor::LaunchBullet()
{
	ProjectileMovement->Velocity += GetActorForwardVector() * ProjectileSpeed;

	float RScalar = FMath::FRandRange(-1.0f, 1.0f) * RotationSpeed;
	FRotator Rando = FMath::VRand().Rotation() * RScalar;
	RotatingMovement->RotationRate = Rando;
}

void ABulletActor::LineTraceForHit()
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

	float DeltaTime = GetWorld()->DeltaTimeSeconds;
	FVector RaycastVector = ProjectileMovement->Velocity * DeltaTime;
	FVector Start = GetActorLocation();
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

	static const FName NAME_MyFName(TEXT("Ammo"));
	if (HitResult && (!Hit.Actor->ActorHasTag(NAME_MyFName)))
	{
		Collide(Hit.GetActor());
	}
}


void ABulletActor::Collide(AActor* OtherActor)
{
	// 1st ignore owner
	if ((MyMechCharacter != nullptr) && (MyTechComponent != nullptr))
	{
		if ((OtherActor != MyTechComponent->GetOwner())
			&& (OtherActor != MyMechCharacter))
		{
			bHit = true;

			// Hacky temp explosion
			///CollisionBox->SetGenerateOverlapEvents(false);
			MeshComp->SetRelativeScale3D(FVector::OneVector * DetonationSize);
			ProjectileMovement->SetVelocityInLocalSpace(FVector::ZeroVector);
			ProjectileMovement->ProjectileGravityScale = 0.77f;
			SetLifeSpan(TimeAfterHit);
			
			if (OtherActor != nullptr)
			{
				AMechCharacter* HitMech = Cast<AMechCharacter>(OtherActor);
				if (HitMech != nullptr)
				{
					if (HitMech->GetLifeSpan() == 0.0f)									/// hacky temp case for "not dead"
					{

						MyMechCharacter->ConfirmHit();

						if (HitMech->IsBot())
						{
							HitMech->DestructMech();
						}
					}
				}
			}
		}
	}
	
}

void ABulletActor::OnBulletBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	static const FName NAME_MyFName(TEXT("Ammo"));
	if (!bHit && !OtherActor->ActorHasTag(NAME_MyFName)) ///bOverlap && 
	{
		Collide(OtherActor);
	}
}

void ABulletActor::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	static const FName NAME_MyFName(TEXT("Ammo"));
	if (!bHit && !OtherActor->ActorHasTag(NAME_MyFName)) ///!bOverlap && 
	{
		Collide(OtherActor);
	}
}

