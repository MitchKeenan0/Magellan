// Fill out your copyright notice in the Description page of Project Settings.

#include "MechAIController.h"


void AMechAIController::BeginPlay()
{
	Super::BeginPlay();

	
}

void AMechAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BotAim(DeltaTime);
	BotMove();
}

void AMechAIController::InitMechBot(AMechCharacter* Mech)
{
	MyMechCharacter = Mech;
	if (MyMechCharacter != nullptr)
	{
		MyMechCharacter->SetIsBot(true);
		
		// Set up mech
		MyMechCharacter->InitOptions();
		MyMechCharacter->TrimOutfit();
		SetControlRotation(MyMechCharacter->GetActorRotation());
		
		// Name
		int NumPotentialNames = BotNames.Num();
		if (NumPotentialNames > 0)
		{
			int Select = FMath::FloorToInt(FMath::FRandRange(0.0f, NumPotentialNames));
			FString NewHandle = BotNames[Select];
			MyMechCharacter->SetMechName(NewHandle);
		}

		UpdateBotPatrol();
		StartBotUpdate();
	}
}

void AMechAIController::StartBotUpdate()
{
	// Random equip select
	int EquipChoice = 0;
	if (FMath::RandRange(0.0f, 1.0f) > 0.5f)
	{
		EquipChoice = 1;
	}

	MyMechCharacter->EquipSelection(EquipChoice);

	UpdateBotPatrol();

	// Bot update
	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, FTimerDelegate::CreateUObject(this, &AMechAIController::UpdateBot, BotUpdatePeriod), BotUpdatePeriod, true);
}

void AMechAIController::StopBotUpdate()
{
	GetWorld()->GetTimerManager().ClearTimer(BotUpdateTimer);
	MyMechCharacter->BotPrimaryTrigger(false);
}

void AMechAIController::UpdateBot(float DeltaTime)
{
	///GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, TEXT("Updating bot..."));
	
	UpdateBotAim(DeltaTime);

	MyMechCharacter->UpdateTorso(DeltaTime); /// this also calls Update Lean for bots

	UpdateBotMovement();

	if (TargetMech != nullptr)
	{
		MyMechCharacter->BotSecondaryTrigger(false);
		
		if (!bBotTriggerDown && (MyMechCharacter->HasLineOfSightTo(TargetMech->GetActorLocation())) && (MyMechCharacter->GetAngleToTarget() < 5.0f))
		{
			float RestTime = GetWorld()->TimeSeconds - TimeAtTriggerUp;
			if (RestTime >= (BotBurstDuration * 0.7f))
			{
				TargetLocation = TargetMech->GetActorLocation();
				if (MyMechCharacter->HasLineOfSightTo(TargetLocation))
				{
					MyMechCharacter->BotPrimaryTrigger(true);
					bBotTriggerDown = true;
					TimeAtTriggerDown = GetWorld()->TimeSeconds;
					BotBurstDuration = FMath::FRandRange(0.2f, 2.0f);
				}
			}
		}

		if (bBotTriggerDown)
		{
			float BurstTime = GetWorld()->TimeSeconds - TimeAtTriggerDown;
			if (BurstTime >= BotBurstDuration)
			{
				bBotTriggerDown = false;
				MyMechCharacter->BotPrimaryTrigger(false);
				TimeAtTriggerUp = GetWorld()->TimeSeconds;
			}
		}

		if (TargetMech->IsDead())
		{
			TargetMech = nullptr;
		}
	}
	else 
	{
		// Patrol
		if (CheckPatrol())
		{
			UpdateBotPatrol();
		}

		if (MyMechCharacter->GetTargetMech() == nullptr)
		{
			MyMechCharacter->BotSecondaryTrigger(true);
		}
		else
		{
			TargetMech = MyMechCharacter->GetTargetMech();
		}
	}

	//// Occluded bots can update slower
	//if (!TorsoCollider->IsVisible() && bVisible)
	//{
	//	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.5f, true, 0.5f); /// this needs its own value
	//	bVisible = false;
	//}
	//else if (TorsoCollider->IsVisible() && !bVisible)
	//{
	//	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.1f, true, 0.1f); /// this needs its own value
	//	bVisible = true;
	//}
}

void AMechAIController::UpdateBotMovement()
{
	// Rotation ingredients
	FVector MyLocation = MyMechCharacter->GetActorLocation();
	FVector MyForward = MyMechCharacter->GetActorForwardVector();
	
	TargetMoveLocation = PatrolMoveVector;
	
	if (TargetMech != nullptr)
	{
		TargetMoveLocation = TargetMech->GetActorLocation();

		float Dist = 1000.0f + FVector::Dist(MyLocation, TargetMech->GetActorLocation());
		float DistToTarget = FMath::Clamp(Dist, 1000.0f, 5000.0f);
		FVector Offset = FMath::VRand() * DistToTarget * Flat;
		TargetMoveLocation += Offset;
	}

	ToTarget = (TargetMoveLocation - MyLocation);
	ToTargetNorm = (ToTarget * Flat).GetSafeNormal();

	// Forward move
	if (MyMechCharacter->GetVelocity().Size() <= FMath::FRandRange(PreferredMoveSpeedMin, PreferredMoveSpeedMax))
	{
		BotMoveValueForward = 1.0f;
	}
	else
	{
		BotMoveValueForward = 0.0f;
	}

	// Strafing move
	if (MyMechCharacter->GetVelocity().Size() < FMath::FRandRange(PreferredMoveSpeedMin, PreferredMoveSpeedMax))
	{
		LateralBias = (ToTarget).GetSafeNormal().Y - MyForward.Y;
		StrafeMoveValue = FMath::Clamp(LateralBias, -1.0f, 1.0f);

		BotMoveValueStrafe = StrafeMoveValue;
	}
	else
	{
		BotMoveValueStrafe = 0.0f;
	}

	// Jump
	if (((TargetMech != nullptr) && !MyMechCharacter->GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.9f))
		|| ((PatrolMoveVector - MyMechCharacter->GetActorLocation()).Z >= 300.0f))
	{
		MyMechCharacter->BotJumpTrigger(true);
	}
	else if (MyMechCharacter->GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.99f))
	{
		MyMechCharacter->BotJumpTrigger(false);
	}

	// Turning move
	/*ToHeadingRight = (MyMechCharacter->GetActorRightVector() * Flat).GetSafeNormal();
	DotToTargetRight = FVector::DotProduct(ToHeadingRight, ToTargetNorm);
	if (FMath::Abs(DotToTargetRight) > 1.0f)
	{
		MoveTurnValue = FMath::Clamp(DotToTargetRight * 100.0f, -1.0f, 1.0f);

		BotMoveValueTurn = MoveTurnValue;
		///MoveTurn(MoveTurnValue);
		LastMoveLateral = MoveTurnValue;

		BotMouseX += (-MoveTurnValue);
	}*/
}

void AMechAIController::BotMove()
{
	MyMechCharacter->MoveForward(BotMoveValueForward);
	MyMechCharacter->MoveRight(BotMoveValueStrafe);
	MyMechCharacter->MoveTurn(BotMoveValueTurn);
}

void AMechAIController::UpdateBotAim(float DeltaTime)
{
	FVector MyLocation = MyMechCharacter->GetActorLocation();
	
	FVector	TargetAimLocation = PatrolLookVector;
	
	if (TargetMech != nullptr)
	{
		TargetAimLocation = TargetMech->GetActorLocation();
	}

	float Distance = FVector::Dist(MyLocation, TargetAimLocation);
	float LeadFactor = 0.0f;
	if (MyMechCharacter->GetEquippedTechActor() != nullptr)
	{
		LeadFactor = MyMechCharacter->GetEquippedTechActor()->GetAimAheadFactor();
	}

	if ((TargetMech != nullptr) && (LeadFactor != 0.0f))
	{
		// Aim ahead
		// Above for gravity
		float DistSqr = FMath::Square(Distance);
		float UnitScale = 160000.0f;
		float ValueByDistance = DistSqr * 0.1f;
		float ExtraForSure = FMath::Sqrt(Distance);
		float VerticalAddition = (ValueByDistance + ExtraForSure) / UnitScale;
		TargetAimLocation.Z += LeadFactor * (VerticalAddition - 100.0f);

		// Ahead for velocity
		FVector PlayerVelocity = TargetMech->GetCharacterMovement()->Velocity * 0.3f;
		float TempScalar = FMath::Clamp(0.1f * FMath::Sqrt(Distance - 150.0f), 0.0001f, 99999.0f);
		TargetAimLocation += LeadFactor * (PlayerVelocity * TempScalar * 0.1f);

		// Velocity offset
		FVector MyVelocity = MyMechCharacter->GetCharacterMovement()->Velocity * 0.16f;
		TargetAimLocation -= (LeadFactor * MyVelocity);
		TargetAimLocation += (PlayerVelocity * LeadFactor);
	}


	// Lllline em up
	USceneComponent* MyAimComponent = MyMechCharacter->GetAimComponent();
	if (MyAimComponent != nullptr)
	{
		FVector TargetNorm = ((TargetAimLocation - MyLocation)).GetSafeNormal();


		// Lateral mouse input
		FVector LateralAim = MyAimComponent->GetRightVector().GetSafeNormal();
		float LateralDot = FVector::DotProduct(LateralAim, TargetNorm);
		float LateralInput = FMath::Clamp(LateralDot * 10.0f, -1.0f, 1.0f);
		BotMouseX = LateralInput; /// FMath::FInterpConstantTo(BotMouseX, LateralInput, DeltaTime, LookSpeed);

		// Target within view
		FVector VerticalAim = MyAimComponent->GetUpVector().GetSafeNormal();
		float VerticalDot = FVector::DotProduct(VerticalAim, TargetNorm);
		float VerticalInput = FMath::Clamp((VerticalDot * 10.0f), -1.0f, 1.0f);
		BotMouseY = VerticalInput; ///FMath::FInterpConstantTo(BotMouseY, VerticalInput, DeltaTime, LookSpeed);
	}
}

void AMechAIController::BotAim(float DeltaTime)
{
	MyMechCharacter->BotAimTo(BotMouseX, BotMouseY);
}


void AMechAIController::UpdateBotPatrol()
{
	// PatrolLook Vector
	// PatrolMove Vector

	/*float GazeRange = 1500000.0f;
	FVector Offset;
	Offset.X = FMath::FRandRange(-GazeRange, GazeRange);
	Offset.Y = FMath::FRandRange(-GazeRange, GazeRange);
	Offset.Z = FMath::FRandRange(-GazeRange, GazeRange);
	Offset.Z = MyMechCharacter->GetAimComponent()->GetComponentLocation().Z + FMath::Clamp(Offset.Z, -100.0f, 100.0f);
	PatrolLookVector = Offset;*/

	float PositionRange = PatrolRange;
	FVector NewPosition;
	NewPosition.X = FMath::FRandRange(-PositionRange, PositionRange);
	NewPosition.Y = FMath::FRandRange(-PositionRange, PositionRange);
	NewPosition.Z = FMath::FRandRange(-1000.0f, 1000.0f);
	PatrolMoveVector = NewPosition;

	PatrolLookVector = PatrolMoveVector;
}

bool AMechAIController::CheckPatrol()
{
	bool Result = false;
	
	bool Looking = false;
	FVector Gaze = (MyMechCharacter->GetAimPoint() - MyMechCharacter->GetActorLocation()).GetSafeNormal();
	FVector NormalPatrol = PatrolLookVector.GetSafeNormal();
	float DotToLook = FVector::DotProduct(Gaze, NormalPatrol);
	if (DotToLook >= 0.88f)
	{
		Looking = true;
	}

	bool Waiting = false;
	float DistToPosition = FVector::Dist(MyMechCharacter->GetActorLocation(), PatrolMoveVector);
	if (DistToPosition <= 10000.0f)
	{
		Waiting = true;
	}

	Result = Looking || Waiting;
	return Result;
}
