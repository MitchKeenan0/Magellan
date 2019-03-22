// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TechComponent.h"
#include "TechActor.generated.h"

class AMechCharacter;

UCLASS()
class MAGELLAN_API ATechActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATechActor();

	UFUNCTION()
	bool IsEquipped();

	UFUNCTION(BlueprintCallable)
	float GetCapacity();

	UFUNCTION()
	void ActivateTech();

	UFUNCTION()
	void DeactivateTech();

	UFUNCTION()
	void InitTechActor(AMechCharacter* TechOwner);

	UFUNCTION()
	UTechComponent* GetTechComponent() { return MyTechComponent; }

	UFUNCTION()
	TSubclassOf<AActor> GetAmmoType() { return AmmoType; }

	UFUNCTION()
	USceneComponent* GetEmitPoint() { return EmitPoint; }

	UFUNCTION()
	FVector GetAimPoint() { return AimPoint; }

	UFUNCTION(BlueprintCallable)
	FName GetTechName() { return TechName; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	FVector AimPoint;

	UFUNCTION()
	void UpdateAimPoint();

	UFUNCTION()
	void UpdateArticulation(float DeltaTime);

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* TechRoot;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* EmitPoint;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* TechMesh;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* TechParticles;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShake> CameraShakeOnActivate;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTechComponent> TechComponentSubclass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTechComponent* MyTechComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName TechName = FName("");

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AmmoType;

	UPROPERTY(EditDefaultsOnly)
	bool bArticulated = true;

	UPROPERTY(EditDefaultsOnly)
	float ArticulationSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float Capacity = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
