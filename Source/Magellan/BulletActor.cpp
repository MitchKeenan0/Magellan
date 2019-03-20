// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletActor.h"

// Sets default values
ABulletActor::ABulletActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->OnComponentBeginOverlap.AddDynamic(this, &ABulletActor::OnBulletBeginOverlap);
	MeshComp->OnComponentHit.AddDynamic(this, &ABulletActor::OnBulletHit);

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ParticleComp->bAbsoluteScale = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = ProjectileSpeed;

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));

	Tags.Add("Ammo");
}

// Called when the game starts or when spawned
void ABulletActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Init Rotation
	float RScalar = FMath::FRandRange(-1.0f, 1.0f) * RotationSpeed;
	FRotator Rando = FMath::VRand().Rotation() * RScalar;
	RotatingMovement->RotationRate = Rando;
}

// Called every frame
void ABulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		}
	}
}


void ABulletActor::Collide(AActor* OtherActor)
{
	if (GetGameTimeSinceCreation() > 0.02f)
	{
		bHit = true;

		// Hacky temp explosion
		MeshComp->SetRelativeScale3D(FVector::OneVector * 5.0f);
		ProjectileMovement->SetVelocityInLocalSpace(FVector::ZeroVector);
		ProjectileMovement->ProjectileGravityScale = 0.33f;
		SetLifeSpan(0.5f);
		
		if (MyMechCharacter != nullptr)
		{
			if (OtherActor != nullptr)
			{
				if (OtherActor->ActorHasTag("Mech"))
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("Hit"));
				}
			}
		}
	}
}

void ABulletActor::OnBulletBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bOverlap && !bHit)
	{
		Collide(OtherActor);
	}
}

void ABulletActor::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bOverlap && !bHit)
	{
		Collide(OtherActor);
	}
}

