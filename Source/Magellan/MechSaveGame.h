// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MechSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class MAGELLAN_API UMechSaveGame : public USaveGame
{
	GENERATED_BODY()

	public:

	// Housekeeping
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UMechSaveGame();


	// Player Mech build
	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool bChanged;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString PlayerName;


	UPROPERTY(VisibleAnywhere, Category = Basic)
	TArray<int32> Hardpoints;


	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 TargetingComputer;
};
