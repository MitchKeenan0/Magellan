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
	void ActivateTech();

	UFUNCTION()
	UTechComponent* GetTechComponent() { return MyTechComponent; }

	UPROPERTY(EditDefaultsOnly)
	FVector TechLocation = FVector::ZeroVector;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void UpdateArticulation(float DeltaTime);

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* TechRoot;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* TechMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UTechComponent* MyTechComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AmmoType;

	UPROPERTY(EditDefaultsOnly)
	bool bArticulated = true;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
