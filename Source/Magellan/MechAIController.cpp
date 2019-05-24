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
	UpdateBotMovement();

	//MyMechCharacter->UpdateAim(DeltaTime);
	MyMechCharacter->UpdateTorso(DeltaTime);

	if ((TargetMech != nullptr) && !TargetMech->IsDead())
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


	}
	else 
	{
		if ((MyMechCharacter->GetTargetMech() == nullptr))/// && (MyMechCharacter->)
		{
			if (FMath::RandRange(0.0f, 1.0f) > 0.8f)
			{
				MyMechCharacter->BotSecondaryTrigger(true);
			}
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
	TargetLocation = MyForward;
	if (TargetMech != nullptr)
	{
		TargetLocation = TargetMech->GetActorLocation();

		float Dist = 5000.0f + FVector::Dist(MyLocation, TargetMech->GetActorLocation());
		float DistToTarget = FMath::Clamp(Dist, 5000.0f, 35000.0f);
		FVector Offset = FMath::VRand() * DistToTarget * Flat;
		TargetLocation += Offset;
	}

	ToTarget = (TargetLocation - MyLocation);
	ToTargetNorm = (ToTarget * Flat).GetSafeNormal();

	// Forward move
	if (MyMechCharacter->HasLineOfSightTo(TargetLocation) || (TargetMech == nullptr))
	{
		if (MyMechCharacter->GetVelocity().Size() < FMath::FRandRange(PreferredMoveSpeedMin, PreferredMoveSpeedMax))
		{
			ForwardMoveValue = FMath::Clamp(ToTarget.Size(), -1.0f, 1.0f);
			if (TargetMech == nullptr)
			{
				ForwardMoveValue *= 0.015f;
			}
			if (FMath::Abs(ForwardMoveValue) > 0.1f)
			{
				BotMoveValueForward = ForwardMoveValue;
			}
		}
		else
		{
			BotMoveValueForward = 0.0f;
		}
	}

	// Strafing move
	else if (MyMechCharacter->GetVelocity().Size() < FMath::FRandRange(PreferredMoveSpeedMin, PreferredMoveSpeedMax))
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
	if ((TargetMech != nullptr) && !MyMechCharacter->GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.9f))
	{
		MyMechCharacter->BotJumpTrigger(true);
	}
	else if (MyMechCharacter->GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.99f))
	{
		MyMechCharacter->BotJumpTrigger(false);
	}

	// Turning move
	ToHeadingRight = (MyMechCharacter->GetActorRightVector() * Flat).GetSafeNormal();
	DotToTargetRight = FVector::DotProduct(ToHeadingRight, ToTargetNorm);
	if (FMath::Abs(DotToTargetRight) > 1.0f)
	{
		MoveTurnValue = FMath::Clamp(DotToTargetRight * 100.0f, -1.0f, 1.0f);

		BotMoveValueTurn = MoveTurnValue;
		///MoveTurn(MoveTurnValue);
		LastMoveLateral = MoveTurnValue;
	}
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
	
	FVector	TargetAimLocation = MyLocation + MyMechCharacter->GetAimComponent()->GetForwardVector();
	if ((TargetMech != nullptr)) {
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
		// Get the angle to target
		FVector TargetNorm = ((TargetAimLocation - MyLocation)).GetSafeNormal();
		FVector LookForward = (MyAimComponent->GetForwardVector()).GetSafeNormal();
		float Dot = FVector::DotProduct(TargetNorm * Flat, LookForward * Flat);
		float RoughAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));
		float YawDirection = FMath::Clamp(MyAimComponent->RelativeRotation.Yaw, -1.0f, 1.0f);
		float AngleToTarget = (RoughAngle * YawDirection) * -1.0f;

		// Lateral mouse input
		/*FVector LateralAim = MyAimComponent->GetRightVector().GetSafeNormal();
		float LateralDot = FVector::DotProduct(LateralAim, TargetNorm);
		float LateralInput = FMath::Clamp(LateralDot * 10.0f, -50.0f, 50.0f);*/
		BotMouseX = FMath::FInterpConstantTo(BotMouseX, AngleToTarget, DeltaTime, LookSpeed); // LateralInput

		// Vertical mouse input
		FVector VerticalAim = MyAimComponent->GetUpVector().GetSafeNormal();
		float VerticalDot = FVector::DotProduct(VerticalAim, TargetNorm);
		float VerticalInput = FMath::Clamp((VerticalDot * 10.0f), -50.0f, 50.0f);
		BotMouseY = FMath::FInterpConstantTo(BotMouseY, VerticalInput, DeltaTime, LookSpeed);
	}
}

void AMechAIController::BotAim(float DeltaTime)
{
	FRotator MRotator = (FRotator(BotMouseY, BotMouseX, 0.0f) * LookSpeed);

	// Clamping
	MRotator.Pitch = FMath::Clamp(MRotator.Pitch, -70.0f, 80.0f);
	MRotator.Roll = MyMechCharacter->GetActorRotation().Roll * -0.5f;

	MyMechCharacter->BotAimTo(MRotator);
}
