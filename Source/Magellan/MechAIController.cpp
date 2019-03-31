// Fill out your copyright notice in the Description page of Project Settings.

#include "MechAIController.h"


void AMechAIController::BeginPlay()
{
	Super::BeginPlay();

	
}

void AMechAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*UpdateLean(DeltaTime);
	UpdateTorso(DeltaTime);

	UpdateTelemetry(DeltaTime);*/
}

void AMechAIController::InitMechBot(AMechCharacter* Mech)
{
	MyMechCharacter = Mech;
	if (MyMechCharacter != nullptr)
	{
		MyMechCharacter->SetIsBot(true);
		
		MyMechCharacter->InitOptions();
		MyMechCharacter->BuildTech(0, 0);
		MyMechCharacter->BuildTech(1, 1);
	}
}