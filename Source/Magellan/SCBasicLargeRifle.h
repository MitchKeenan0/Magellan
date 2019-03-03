// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SCBasicLargeRifle.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGELLAN_API USCBasicLargeRifle : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USCBasicLargeRifle();

	void StartFire();
	void EndFire();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RifleMesh;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> AmmoType;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
