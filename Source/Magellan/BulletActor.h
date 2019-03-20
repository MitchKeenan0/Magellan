// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Magellan.h"
#include "GameFramework/Actor.h"
#include "TechComponent.h"
#include "MechCharacter.h"
#include "BulletActor.generated.h"

UCLASS()
class MAGELLAN_API ABulletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void InitBullet(UTechComponent* Shooter);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Collide();

	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpeed = 35000.0f;

	UPROPERTY(EditDefaultsOnly)
	float RotationSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	bool bOverlap = false;

	UPROPERTY(BlueprintReadOnly)
	UTechComponent* MyTechComponent;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UParticleSystemComponent* ParticleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	URotatingMovementComponent* RotatingMovement;

	UFUNCTION()
	void OnBulletBeginOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnBulletHit
	(
		UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
	);
	

private:	
	UPROPERTY()
	bool bHit = false;

};
