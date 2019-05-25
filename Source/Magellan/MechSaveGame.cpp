// Fill out your copyright notice in the Description page of Project Settings.


#include "MechSaveGame.h"


UMechSaveGame::UMechSaveGame()
{
	SaveSlotName = TEXT("MechSaveSlot");
	UserIndex = 0;

	PlayerName = TEXT("Unnamed");
	
	Hardpoints.Init(0, 2);

	TargetingComputer = 0;

	bChanged = false;
}