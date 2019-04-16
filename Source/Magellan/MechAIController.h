// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Magellan.h"
#include "AIController.h"
#include "MechCharacter.h"
#include "MechAIController.generated.h"

/**
 * 
 */
UCLASS()
class MAGELLAN_API AMechAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InitMechBot(AMechCharacter* Mech);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BotNames;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter = nullptr;
	
private:
	UPROPERTY()
	float MyInputX = 0.0f;

	UPROPERTY()
	float MyInputZ = 0.0f;

	UPROPERTY()
	FVector TargetVector = FVector::ZeroVector;

	UPROPERTY()
	AMechCharacter* TargetMech = nullptr;

};
