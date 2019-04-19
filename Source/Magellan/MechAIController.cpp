// Fill out your copyright notice in the Description page of Project Settings.

#include "MechAIController.h"


void AMechAIController::BeginPlay()
{
	Super::BeginPlay();

	
}

void AMechAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMechAIController::InitMechBot(AMechCharacter* Mech)
{
	MyMechCharacter = Mech;
	if (MyMechCharacter != nullptr)
	{
		MyMechCharacter->SetIsBot(true);
		
		// Set up tech
		MyMechCharacter->InitOptions();
		MyMechCharacter->TrimOutfit();
		MyMechCharacter->StartBotUpdate();

		SetControlRotation(MyMechCharacter->GetActorRotation());

		// Name
		int NumPotentialNames = BotNames.Num();
		if (NumPotentialNames > 0)
		{
			int Select = FMath::FloorToInt(FMath::FRandRange(0.0f, NumPotentialNames));
			FString NewHandle = BotNames[Select];
			MyMechCharacter->SetMechName(NewHandle);
		}
	}
}