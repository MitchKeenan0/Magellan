// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TechComponent.generated.h"

UCLASS()
class MAGELLAN_API ATechComponent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATechComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* TechRoot;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* TechMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};