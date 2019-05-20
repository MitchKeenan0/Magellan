// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	// Propterties
	UPROPERTY(EditAnywhere)
	float LookSpeed = 0.3f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BotNames;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InitMechBot(AMechCharacter* Mech);

	UFUNCTION()
	void StartBotUpdate();

	UFUNCTION()
	void StopBotUpdate();

	UFUNCTION(BlueprintCallable)
	void UpdateBot(float DeltaTime);

	UFUNCTION()
	void UpdateBotMovement();

	UFUNCTION()
	void BotMove();

	UFUNCTION()
	void UpdateBotAim(float DeltaTime);

	UFUNCTION()
	void BotAim(float DeltaTime);

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle BotUpdateTimer;

	UPROPERTY(BlueprintReadOnly)
	AMechCharacter* MyMechCharacter = nullptr;
	
private:

	UPROPERTY()
	float MyInputX = 0.0f;

	UPROPERTY()
	float MyInputZ = 0.0f;

	UPROPERTY()
	float BotMouseX = 0.0f;

	UPROPERTY()
	float BotMouseY = 0.0f;

	UPROPERTY()
	FVector TargetVector = FVector::ZeroVector;

	UPROPERTY()
	AMechCharacter* TargetMech = nullptr;


	UPROPERTY()
	float BotMoveValueForward = 0.0f;

	UPROPERTY()
	float BotMoveValueStrafe = 0.0f;

	UPROPERTY()
	float BotMoveValueTurn = 0.0f;

	UPROPERTY()
	float LastMoveLateral = 0.0f;

	UPROPERTY()
	bool bBotTriggerDown = false;

	UPROPERTY()
	float TimeAtTriggerDown = 0.0f;

	UPROPERTY()
	float TimeAtTriggerUp = 0.0f;

	UPROPERTY()
	float BotBurstDuration = 1.0f;


	// Movement
	UPROPERTY()
	FVector Flat = FVector(1.0f, 1.0f, 0.0f);

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector ToTarget = FVector::ZeroVector;

	UPROPERTY()
	FVector ToTargetNorm = FVector::ZeroVector;

	UPROPERTY()
	FVector ToHeadingRight = FVector::ZeroVector;

	UPROPERTY()
	float ForwardMoveValue = 0.0f;

	UPROPERTY()
	float LateralBias = 0.0f;

	UPROPERTY()
	float StrafeMoveValue = 0.0f;

	UPROPERTY()
	float DotToTargetRight = 0.0f;

	UPROPERTY()
	float MoveTurnValue = 0.0f;

};
