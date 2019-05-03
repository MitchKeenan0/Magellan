// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacter.h"
#include "TargetingTechComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AMechCharacter::AMechCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AimComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AimComponent"));
	AimComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	TorsoCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TorsoCollider"));
	TorsoCollider->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TorsoCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	LegCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("LegCollider"));
	LegCollider->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	Torso->AttachToComponent(TorsoCollider, FAttachmentTransformRules::KeepRelativeTransform);

	Outfit = CreateDefaultSubobject<UMechOutfitComponent>(TEXT("Outfit"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->AttachToComponent(AimComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SpringArmComp->SetRelativeLocation(FVector(10.0f, 0.0f, -10.0f));
	SpringArmComp->TargetArmLength = -20.0f;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 10.0f;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraRotationLagSpeed = 8.0f;
	SpringArmComp->CameraLagMaxDistance = 15.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepRelativeTransform);
	CameraComp->FieldOfView = 100.0f;

	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	JumpMaxHoldTime = MaxJumpTime;

	bUseControllerRotationYaw = false;

	OnBrakeDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	OnDodgeDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	OnLiftDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	//OnTelemetryDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	//OnHitDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
}

void AMechCharacter::SetMechName(FString Value)
{
	MechName = Value;
}

void AMechCharacter::TestFunction(bool bOn)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Delegate Received"));
}

// Called when the game starts or when spawned
void AMechCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	MyHealth = MaxHealth;
	
	GetCharacterMovement()->MaxWalkSpeed = TopSpeed;

	EquipSelection(-1.0f);

	// Timer for update
	GetWorld()->GetTimerManager().SetTimer(PlayerUpdateTimer, this, &AMechCharacter::UpdatePlayer, PlayerUpdateRate, true, 0.1f);

	// Targeting timer
	GetWorld()->GetTimerManager().SetTimer(TargetUpdateTimer, this, &AMechCharacter::UpdateTargets, 0.05f, true, 1.1f);
}

void AMechCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void AMechCharacter::DestructMech()
{
	if ((GetController() != nullptr) && !bDead)
	{
		// Player death
		if (!bCPU)
		{
			SpringArmComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			
			if (MyHud != nullptr)
			{
				MyHud->RemoveFromViewport();
				MyHud->Destruct();
				MyHud = nullptr;
			}

			if (MyTargeter != nullptr)
			{
				MyTargeter->RemoveFromViewport();
				MyTargeter->Destruct();
				MyTargeter = nullptr;
			}

			APlayerController* MyPlayerController = Cast<APlayerController>(GetController());
			if (MyPlayerController != nullptr)
			{
				MyPlayerController->SetInputMode(FInputModeUIOnly());
				MyPlayerController->bShowMouseCursor = true;

				// Death screen
				if (DeathWidgetClass != nullptr)
				{
					MyDeathScreen = CreateWidget<UUserWidget>(MyPlayerController, DeathWidgetClass);
					if (MyDeathScreen)
					{
						MyDeathScreen->AddToViewport();
						MyDeathScreen->SetOwningPlayer(MyPlayerController);
					}
				}
			}
		}

		// Clear up
		EndScope();
		PrimaryStopFire();
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		bDead = true;

		// "Terminal velocity"
		EndJump();
		FVector LastVelocity = GetCharacterMovement()->Velocity;
		LaunchCharacter(LastVelocity, true, true);
		GetCharacterMovement()->GroundFriction *= 5.0f;

		// Break up components
		if (Outfit != nullptr)
		{
			Outfit->ClearOutfit();
		}

		// Explode & physics
		SpringArmComp->bEnableCameraRotationLag = true;
		SpringArmComp->CameraRotationLagSpeed *= 0.1f;
		TorsoCollider->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TorsoCollider->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		TorsoCollider->SetSimulatePhysics(true);
		TorsoCollider->WakeRigidBody();
		GetMesh()->AddRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		
		LegCollider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		FVector Offset = (FMath::VRand() * 100.0f);
		if (Offset.Z < 0.0f) {
			Offset.Z *= -1.0f;
		}

		TorsoCollider->AddImpulse((LastVelocity * 300.0f) + (Offset * 2500.0f));
		TorsoCollider->AddTorqueInRadians(Offset * 100.0f);

		// Kill bot
		if (bCPU)
		{
			StopBotUpdate();
			SetLifeSpan(3.0f);
			GetController()->Destroy();
		}

		// Particle fx
		if (DestructParticles != nullptr)
		{
			FVector Location = GetActorLocation();
			FRotator Rotation = GetActorForwardVector().Rotation();
			MyDestructParticles = UGameplayStatics::SpawnEmitterAttached(DestructParticles, GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepWorldPosition, true);
		}
	}
}

void AMechCharacter::InitMech()
{
	APlayerController* MyPlayerCtrl = Cast<APlayerController>(GetController());
	if (MyPlayerCtrl)
	{
		MyHud = CreateWidget<UUserWidget>(MyPlayerCtrl, HudWidgetClass);
		if (MyHud)
		{
			MyHud->AddToViewport();
			MyHud->SetOwningPlayer(MyPlayerCtrl);
		}

		MyTargeter = CreateWidget<UUserWidget>(MyPlayerCtrl, TargeterWidgetClass);
		if (MyTargeter)
		{
			MyTargeter->AddToViewport();
			MyTargeter->SetOwningPlayer(MyPlayerCtrl);
		}
	}
	
	Torso->SetOwnerNoSee(true);
	GetMesh()->SetOwnerNoSee(true);

	TrimOutfit();
	OffsetCamera(FVector::ZeroVector, FRotator::ZeroRotator, PlayerFOV);
	GetController()->SetControlRotation(GetActorRotation());

	FOutputDraw IdleDraw;
	OutputDraws.Init(IdleDraw, 1);
}

void AMechCharacter::StartBotUpdate()
{
	// Random equip select
	int EquipChoice = 0;
	if (FMath::RandRange(0.0f, 1.0f) > 0.5f)
	{
		EquipChoice = 1;
	}
	EquipSelection(EquipChoice);

	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.01f, true, 0.01f); /// this needs its own value
}
void AMechCharacter::StopBotUpdate()
{
	GetWorld()->GetTimerManager().ClearTimer(BotUpdateTimer);
	PrimaryStopFire();
}

// Called every frame
void AMechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bDead)
	{
		// These need the smoothest of values ;p
		UpdateLean(DeltaTime);
		UpdateTorso(DeltaTime);

		if (!bCPU)
		{
			UpdateAim(DeltaTime);
			TelemetryTimer += DeltaTime;
		}
		else
		{
			BotMove();
		}
	}
}

void AMechCharacter::UpdatePlayer()
{
	if ((Controller != nullptr) && (Controller->IsLocalController()))
	{
		if (!bCPU)
		{
			float DeltaTime = GetWorld()->DeltaTimeSeconds;
			UpdateTelemetry(DeltaTime);
		}
	}
}

// Called to bind functionality to input
void AMechCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Actions
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMechCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMechCharacter::EndJump);
	PlayerInputComponent->BindAction("Brake", IE_Pressed, this, &AMechCharacter::StartBrake);
	PlayerInputComponent->BindAction("Brake", IE_Released, this, &AMechCharacter::EndBrake);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &AMechCharacter::PrimaryFire);
	PlayerInputComponent->BindAction("PrimaryFire", IE_Released, this, &AMechCharacter::PrimaryStopFire);
	PlayerInputComponent->BindAction("SecondaryFire", IE_Pressed, this, &AMechCharacter::SecondaryFire);
	PlayerInputComponent->BindAction("SecondaryFire", IE_Released, this, &AMechCharacter::SecondaryStopFire);
	PlayerInputComponent->BindAction("Centre", IE_Pressed, this, &AMechCharacter::CentreMech);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMechCharacter::Dodge);
	PlayerInputComponent->BindAction("Scope", IE_Pressed, this, &AMechCharacter::StartScope);
	PlayerInputComponent->BindAction("Scope", IE_Released, this, &AMechCharacter::EndScope);

	// Axes
	PlayerInputComponent->BindAxis("MoveRight", this, &AMechCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMechCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveTurn", this, &AMechCharacter::MoveTurn);
	PlayerInputComponent->BindAxis("EquipSelect", this, &AMechCharacter::EquipSelection);
}

// Movement
void AMechCharacter::MoveRight(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar);

	/// for dodge direction
	LastMoveLateral = Value;
}
void AMechCharacter::MoveForward(float Value)
{
	AddMovementInput(GetMesh()->GetForwardVector(), Value * MoveSpeed);

	LastMoveForward = Value;

	// Bring legs around towards look direction
	if (Value != 0.0f)
	{
		float LegsAngle = GetLegsToTorsoAngle();
		float AlignSpeed = FMath::Abs(LegsAngle) * 0.01f;
		if (LegsAngle < -1.0f)
		{
			MoveTurn(AlignSpeed);
		}
		else if (LegsAngle > 1.0f)
		{
			MoveTurn(-AlignSpeed);
		}
	}
}
void AMechCharacter::MoveTurn(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar * 0.0001f);
	
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += (Value * TurnSpeed);

	// Turning and Aim counter-rotation
	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, GetWorld()->DeltaTimeSeconds, TurnSpeed);
	FRotator DeltaRotation = InterpRotation - GetActorRotation();
	DeltaRotation.Roll = 0.0f;

	SetActorRotation(InterpRotation);
	AimComponent->AddRelativeRotation(DeltaRotation * -1.0f);

	if (LastMoveLateral == 0.0f)
	{
		/// For lean
		LastMoveLateral = Value;
	}
}

// Jump
void AMechCharacter::StartJump()
{
	if (OnLiftDelegate.IsBound())
	{
		OnLiftDelegate.Broadcast(true);
	}

	EndBrake();
	Jump();
}
void AMechCharacter::EndJump()
{
	if (OnLiftDelegate.IsBound())
	{
		OnLiftDelegate.Broadcast(false);
	}

	StopJumping();
}

// Brake
void AMechCharacter::StartBrake()
{
	if (OnBrakeDelegate.IsBound())
	{
		OnBrakeDelegate.Broadcast(true);
	}

	//EndJump();
	
	GetCharacterMovement()->MaxWalkSpeed = BrakeStrength;
	GetCharacterMovement()->GroundFriction = 10.0f;
	
	if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->BrakingDecelerationFalling = GetCharacterMovement()->Velocity.Size();
	}
	
	bBraking = true;
}

void AMechCharacter::EndBrake()
{
	if (OnBrakeDelegate.IsBound())
	{
		OnBrakeDelegate.Broadcast(false);
	}

	GetCharacterMovement()->MaxWalkSpeed = TopSpeed;
	GetCharacterMovement()->GroundFriction = 2.0f;
	
	if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->BrakingDecelerationFalling = 50.0f;
	}
	
	bBraking = false;
}

void AMechCharacter::Dodge()
{
	if (OnDodgeDelegate.IsBound())
	{
		OnDodgeDelegate.Broadcast(true);
	}

	// Relative to player
	FVector ForwardV = GetMesh()->GetForwardVector() * LastMoveForward;
	FVector LateralV = GetMesh()->GetRightVector() * LastMoveLateral;
	FVector DodgeVector = (ForwardV + LateralV);
	
	/// big jump potential
	if (LateralV == FVector::ZeroVector)
	{
		DodgeVector += (FVector::UpVector * 0.1f);

		/// leap!
		if (ForwardV == FVector::ZeroVector)
		{
			DodgeVector.Z *= 7.5f;
		}
	}
	
	DodgeVector *= DodgeSpeed;
	
	LaunchCharacter(DodgeVector, false, false);
}

void AMechCharacter::CentreMech()
{
	if (TorsoCollider != nullptr)
	{
		TorsoCollider->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (AimComponent != nullptr)
	{
		AimComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}

	if (Outfit != nullptr)
	{
		int NumTechs = Outfit->HardpointTechs.Num();
		if (NumTechs > 0)
		{
			for (int i = 0; i < NumTechs; ++i)
			{
				ATechActor* ThisTech = Outfit->HardpointTechs[i];
				if (ThisTech != nullptr)
				{
					ThisTech->SetActorRotation(TorsoCollider->GetComponentRotation());
				}
			}
		}
	}
}

void AMechCharacter::StartScope()
{
	CameraComp->FieldOfView = ScopeFOV;
}

void AMechCharacter::EndScope()
{
	CameraComp->FieldOfView = PlayerFOV;
}

void AMechCharacter::EquipSelection(float Value)
{
	if (Value != 0.0f)
	{
		PrimaryStopFire();
		
		EquipSelectValue = Value;

		int MaxSafeValue = Outfit->HardpointTechs.Num() - 1;
		if (EquipSelectValue > MaxSafeValue)
		{
			EquipSelectValue = MaxSafeValue;
		}
		if (EquipSelectValue < 0)
		{
			EquipSelectValue = 0;
		}
	}
}

FName AMechCharacter::GetEquippedTechName()
{
	FName Result = FName("None");
	if (Outfit->HardpointTechs.Num() >= (EquipSelectValue + 1))
	{
		ATechActor* MyPrimaryTech = Outfit->HardpointTechs[EquipSelectValue];
		if (MyPrimaryTech != nullptr)
		{
			Result = MyPrimaryTech->GetTechName();
		}
	}
	return Result;
}

ATechActor* AMechCharacter::GetTechActor(int EquipIndex)
{
	ATechActor* Result = nullptr;

	if (Outfit->HardpointTechs.Num() >= (EquipIndex + 1))
	{
		ATechActor* MyTech = Outfit->HardpointTechs[EquipIndex];
		if (MyTech != nullptr)
		{
			Result = MyTech;
		}
	}

	return Result;
}

ATechActor* AMechCharacter::GetEquippedTechActor()
{
	ATechActor* Result = nullptr;

	ATechActor* MyTech = GetTechActor(EquipSelectValue);
	if (MyTech != nullptr)
	{
		Result = MyTech;
	}

	return Result;
}

void AMechCharacter::UpdateAim(float DeltaTime)
{
	// Get Mouse inputs
	float X;
	float Y;

	if (bCPU)
	{
		X = BotMouseX;
		Y = BotMouseY;
	}
	else
	{
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputMouseDelta(X, Y);
	}

	// Aim rotation
	FRotator MRotator = FRotator(Y, X, 0.0f) * CameraSensitivity;
	AimComponent->AddRelativeRotation(MRotator);
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	FRotator Current = AimComponent->GetRelativeTransform().Rotator();
	AimRotation = FMath::RInterpTo(Current, AimRotation, DeltaTime, CameraSensitivity * 10.0f);
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -70.0f, 80.0f);
	
	AimComponent->SetRelativeRotation(AimRotation);
}

void AMechCharacter::UpdateTorso(float DeltaTime)
{
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	FRotator CurrentR = TorsoCollider->GetRelativeTransform().Rotator();
	FRotator InterpRotator = FMath::RInterpTo(CurrentR, AimRotation, DeltaTime, TorsoSpeed);
	InterpRotator.Pitch = FMath::Clamp(InterpRotator.Pitch, TorsoMinPitch, TorsoMaxPitch);
	
	TorsoCollider->SetRelativeRotation(InterpRotator);
}

void AMechCharacter::UpdateLean(float DeltaTime)
{
	FRotator Lean = FRotator::ZeroRotator;

	// Setting up angular relations
	FVector MyVelocity = GetCharacterMovement()->Velocity.GetSafeNormal();
	FVector MyForward = GetActorForwardVector().GetSafeNormal();
	float DotToVelocityForward = FVector::DotProduct(MyVelocity, MyForward);
	FVector MyRight = GetActorRightVector().GetSafeNormal();
	float DotToVelocityRight = FVector::DotProduct(MyVelocity, MyRight);

	// Initial rotation
	Lean.Pitch = DotToVelocityForward * -MoveTilt;
	Lean.Yaw = GetActorRotation().Yaw;
	Lean.Roll = LastMoveLateral * MoveTilt;

	// Velocity mapped 0.0 -- 1.0
	float a1 = 1.0f;
	float a2 = TopSpeed;
	float s = GetCharacterMovement()->Velocity.Size();
	float b1 = 0.01f;
	float b2 = 1.0f;
	float t = b1 + (((s - a1) * (b2 - b1)) / (a2 - a1));
	float ScaledVelocity = FMath::Sqrt(t);

	Lean.Pitch *= ScaledVelocity;
	Lean.Roll *= ScaledVelocity;

	// Braking response
	if (bBraking && (GetCharacterMovement()->Velocity.Size() > 5.0f))
	{
		Lean.Pitch *= -1.0f;
		//Lean.Roll *= -1.618f;
		Lean.Roll = DotToVelocityRight * MoveTilt * ScaledVelocity * -1.618f;
	}

	FRotator TargetRotation = Lean;
	FRotator InterpLean = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, TorsoSpeed);

	FRotator DeltaRotation = InterpLean - GetActorRotation();
	DeltaRotation.Roll = 0.0f;

	SetActorRotation(InterpLean);
	AimComponent->AddRelativeRotation(DeltaRotation * -1.0f);
}

void AMechCharacter::UpdateTelemetry(float DeltaTime)
{
	if (TelemetryTimer >= (1.0f / TelemetryUpdateRate))
	{
		TelemetryTimer = 0.0f;

		if (OnTelemetryDelegate.IsBound())
		{
			float Velocity = GetCharacterMovement()->Velocity.Size() * 0.034f;
			float Altitude = GetAltitude();
			bool bAirborne = (Altitude > 5.0f); /// GetCharacterMovement()->IsFalling();
			OnTelemetryDelegate.Broadcast(bAirborne, Velocity, Altitude);
		}
	}
}

float AMechCharacter::GetAltitude()
{
	float Result = 0.0f;

	bool HitResult = false;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	FHitResult Hit;
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this->GetOwner());

	FVector RaycastVector = (FVector::UpVector * -99000.0f);
	FVector Start = GetActorLocation();
	FVector End = Start + RaycastVector;

	// Raycast down
	HitResult = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		TraceObjects,
		false,
		IgnoredActors,
		EDrawDebugTrace::None,
		Hit,
		true,
		FLinearColor::White, FLinearColor::Red, 0.5f);

	if (HitResult)
	{
		float Distance = FVector::Dist(GetActorLocation(), Hit.ImpactPoint) * 0.02f;
		if (FMath::Abs(Distance) > 4.0f)
		{
			Result = Distance;
		}
	}

	return Result;
}

FVector AMechCharacter::GetLookVector()
{
	// Initial aim, in case ray hits nothing
	FVector Result = AimComponent->GetComponentLocation() + (AimComponent->GetForwardVector() * 550000.0f);

	// Linecast ingredients
	bool HitResult = false;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible));
	FHitResult Hit;
	
	// Ignore Torso & Tech
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(TorsoCollider->GetOwner());
	int NumTechs = Outfit->HardpointTechs.Num();
	if (NumTechs > 0)
	{
		for (int i = 0; i < NumTechs; ++i)
		{
			if (Outfit->HardpointTechs[i] != nullptr)
			{
				IgnoredActors.Add(Outfit->HardpointTechs[i]);
			}
		}
	}
	
	// Pew pew
	FVector RaycastVector = AimComponent->GetForwardVector() * 50000.0f;
	FVector Start = AimComponent->GetComponentLocation();
	FVector End = Start + RaycastVector;

	HitResult = UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		TraceObjects,
		false,
		IgnoredActors,
		EDrawDebugTrace::None,
		Hit,
		true,
		FLinearColor::Red, FLinearColor::Red, 5.0f);

	static const FName NAME_MyFName(TEXT("Ammo"));
	if (HitResult && !(Hit.Actor->ActorHasTag(NAME_MyFName)))
	{
		Result = Hit.ImpactPoint;
	}

	return Result;
}

FVector AMechCharacter::GetAimPoint()
{
	FVector Result = FVector::ZeroVector;
	
	if (Outfit->HardpointTechs.Num() >= (EquipSelectValue + 1))
	{
		ATechActor* MyPrimaryTech = GetTechActor(EquipSelectValue);
		if (MyPrimaryTech != nullptr)
		{
			Result = MyPrimaryTech->GetAimPoint();
		}
	}
	
	if (Result == FVector::ZeroVector)
	{
		Result = GetActorLocation() + (CameraComp->GetForwardVector() * 100000.0f);
	}

	return Result;
}

FVector AMechCharacter::GetTorsoPoint()
{
	FVector TorsoDirection = TorsoCollider->GetForwardVector() * 20000.0f;
	FVector Result = GetActorLocation() + TorsoDirection;
	return Result;
}

float AMechCharacter::GetLegsToTorsoAngle()
{
	// Collapses the vectors onto the world plane and returns angle in world space
	FVector LegsForward = ( GetMesh()->GetForwardVector() * FVector(1.0f, 1.0f, 0.0f) ).GetSafeNormal();
	FVector TorsoForward = ( TorsoCollider->GetForwardVector() * FVector(1.0f, 1.0f, 0.0f) ).GetSafeNormal();
	float Dot = FVector::DotProduct(LegsForward, TorsoForward);
	float RoughAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));
	float YawDirection = FMath::Clamp(TorsoCollider->RelativeRotation.Yaw, -1.0f, 1.0f);
	float Result = (RoughAngle * YawDirection) * -1.0f;
	return Result;
}

void AMechCharacter::PrimaryFire()
{
	ATechActor* MyPrimaryTech = GetTechActor(EquipSelectValue);
	if (MyPrimaryTech != nullptr)
	{
		MyPrimaryTech->ActivateTech();
	}
}

void AMechCharacter::PrimaryStopFire()
{
	ATechActor* MyPrimaryTech = GetTechActor(EquipSelectValue);
	if (MyPrimaryTech != nullptr)
	{
		MyPrimaryTech->DeactivateTech();
	}
}

void AMechCharacter::SecondaryFire()
{
	if (TargetingComputer != nullptr)
	{
		TargetingComputer->ActivateTechComponent();
	}
}

void AMechCharacter::SecondaryStopFire()
{
	if (TargetingComputer != nullptr)
	{
		TargetingComputer->DeactivateTechComponent();
	}
}

void AMechCharacter::BuildTech(int TechID, int TechHardpoint)
{
	if (AvailableTechPointers.Num() >= (TechID + 1))
	{
		ATechActor* NewTechType = AvailableTechPointers[TechID];
		if (NewTechType != nullptr)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[TechID], SpawnInfo);
			if (NewTech != nullptr)
			{
				Outfit->HardpointTechs.Insert(NewTech, TechHardpoint);
				
				///GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Fitted new tech"));

				NewTech->AttachToComponent(TorsoCollider, FAttachmentTransformRules::KeepRelativeTransform);
				
				FVector SetLocation = Outfit->HardpointLocations[TechHardpoint];
				NewTech->SetActorRelativeLocation(SetLocation);

				NewTech->SetActorRelativeRotation(FRotator::ZeroRotator);

				NewTech->InitTechActor(this);
			}
		}
	}
}

void AMechCharacter::InitOptions()
{
	int numTechs = AvailableTech.Num();
	if (numTechs > 0)
	{
		Outfit->HardpointTechs.Init(nullptr, numTechs);
		//AvailableTechPointers.Init(nullptr, numTechs);

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int i = 0; i < numTechs; ++i)
		{
			if (AvailableTech[i] != nullptr)
			{
				ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[i], SpawnInfo);
				if (NewTech != nullptr)
				{
					AvailableTechPointers.Insert(NewTech, i);

					if (i < 2) /// Replace 2 with actual Hardpoints
					{
						BuildTech(i, i);
					}
				}
			}
		}
	}

	// Targeting
	int NumTargeters = TargetingTech.Num();
	if (NumTargeters > 0)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (TargetingTech[0] != nullptr)
		{
			TargetingComputer = NewObject<UTargetingTechComponent>(this, *TargetingTech[0]);
			if (TargetingComputer != nullptr)
			{
				TargetingComputer->RegisterComponent();
				TargetingComputer->InitTechComponent(this, -1.0f);
				TargetingComputer->EmitPoint = AimComponent;
				
				///MyTechComponent->SetParticles(TechParticles);
			}
		}
	}
}

TArray<ATechActor*> AMechCharacter::GetBuilderTechByTag(FName Tag)
{
	TArray<ATechActor*> Result;
	
	int numTechPtrs = AvailableTechPointers.Num();
	if (numTechPtrs > 0)
	{
		// Compare against existing tech
		for (int i = 0; i < numTechPtrs; ++i)
		{
			ATechActor* ThisTech = AvailableTechPointers[i];
			if ((ThisTech != nullptr) && ThisTech->ActorHasTag(Tag))
			{
				Result.Add(AvailableTechPointers[i]);
			}
		}
	}

	return Result;
}

void AMechCharacter::OffsetCamera(FVector Offset, FRotator Rotation, float FOV)
{
	SpringArmComp->SetRelativeLocation(Offset);
	SpringArmComp->SetRelativeRotation(Rotation);
	CameraComp->FieldOfView = FOV;
}

void AMechCharacter::TrimOutfit()
{
	int NumTechs = AvailableTechPointers.Num();
	if (NumTechs > 0)
	{
		for (int i = 0; i < NumTechs; ++i)
		{
			if (AvailableTechPointers[i] != nullptr)
			{
				AvailableTechPointers[i]->Destroy();
			}
		}

		AvailableTechPointers.Empty();
	}
}

void AMechCharacter::RemovePart(int TechID, int HardpointIndex)
{
	int NumTechs = Outfit->HardpointTechs.Num();
	if (NumTechs >= (HardpointIndex + 1))
	{
		if (Outfit->HardpointTechs[HardpointIndex] != nullptr)
		{
			Outfit->HardpointTechs[HardpointIndex]->Destroy();
			Outfit->HardpointTechs.RemoveAt(HardpointIndex);
		}
	}
}

void AMechCharacter::ConfirmHit()
{
	if (!bCPU)
	{
		if (OnHitDelegate.IsBound())
		{
			OnHitDelegate.Broadcast();
		}
	}
}

float AMechCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	MyHealth -= Damage;

	if (MyHealth <= 0.0f)
	{
		DestructMech();
	}

	///GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("OUCH"));

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AMechCharacter::ReceiveLock()
{
	if (!bCPU && OnReceiveLockDelegate.IsBound())
	{
		OnReceiveLockDelegate.Broadcast();

		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, TEXT("OH SHIT SON U GOT LOCKED"));
	}
}

void AMechCharacter::UpdateTargets()
{
	if (TargetingComputer != nullptr)
	{
		
		LockedTargets = TargetingComputer->GetLockedTargets();
		
		// For bot
		if ((LockedTargets.Num() > 0) && (LockedTargets[0] != nullptr))
		{
			TargetMech = Cast<AMechCharacter>(LockedTargets[0]);
		}
		
		// Hud update
		if (OnTargetLockDelegate.IsBound())
		{
			OnTargetLockDelegate.Broadcast();
		}
	}
}

void AMechCharacter::UpdateBot()
{
	// Get target
	SecondaryFire();

	if ((TargetMech != nullptr) && !TargetMech->IsDead())
	{
		float DeltaTime = GetWorld()->DeltaTimeSeconds;
		
		UpdateBotAim(DeltaTime);
		UpdateBotMovement();

		if (!bBotTriggerDown && (HasLineOfSightTo(TargetMech->GetActorLocation())) && (GetAngleToTarget() < 5.0f))
		{
			float RestTime = GetWorld()->TimeSeconds - TimeAtTriggerUp;
			if (RestTime >= (BotBurstDuration * 0.7f))
			{
				TargetLocation = TargetMech->GetActorLocation();
				if (HasLineOfSightTo(TargetLocation))
				{
					PrimaryFire();
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
				PrimaryStopFire();
				TimeAtTriggerUp = GetWorld()->TimeSeconds;
			}
		}

		
	}
	else
	{
		if (LockedTargets.Num() < 1)
		{
			TArray<AActor*> Mechs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMechCharacter::StaticClass(), Mechs);
			int MechNum = Mechs.Num();
			if (MechNum > 0)
			{
				int Rando = FMath::FloorToInt(FMath::FRandRange(0.0f, (MechNum - 1.0f)));
				AMechCharacter* MechC = Cast<AMechCharacter>(Mechs[Rando]);
				if (MechC != nullptr)
				{
					// Check target is valid
					if ((MechC != this) && !MechC->IsDead())
					{
						// Check team
						if (MechC->GetTeam() != TeamID)
						{
							TargetMech = MechC;
						}
					}
				}
			}
		}
		
		// unused- Get Player
		//TargetMech = Cast<AMechCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	// Occluded bots can update slower
	if (!TorsoCollider->IsVisible() && bVisible)
	{
		GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.1f, true, 0.1f); /// this needs its own value
		bVisible = false;
	}
	else if (TorsoCollider->IsVisible() && !bVisible)
	{
		GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.01f, true, 0.01f); /// this needs its own value
		bVisible = true;
	}

	SecondaryStopFire(); // ???
}

void AMechCharacter::UpdateBotMovement()
{
	// Rotation ingredients
	Flat = FVector(1.0f, 1.0f, 0.0f);
	TargetLocation = GetActorForwardVector();
	if (TargetMech != nullptr)
	{
		TargetLocation = TargetMech->GetActorLocation();
	}
	ToTarget = (TargetLocation - GetActorLocation());
	ToTargetNorm = (ToTarget * Flat).GetSafeNormal();
	
	// Forward move
	if (HasLineOfSightTo(TargetLocation)
		&& (ToTarget.Size() >= 7000.0f))
	{
		ForwardMoveValue = FMath::Clamp(ToTarget.Size(), -1.0f, 1.0f);
		if (FMath::Abs(ForwardMoveValue) > 0.25f)
		{
			
			BotMoveValueForward = ForwardMoveValue;
			///MoveForward(ForwardMoveValue);
		}
	}

	// Strafing move
	else
	{
		LateralBias = (ToTarget).GetSafeNormal().Y - GetActorForwardVector().Y;
		StrafeMoveValue = FMath::Clamp(LateralBias, -1.0f, 1.0f);
		
		BotMoveValueStrafe = StrafeMoveValue;
		///MoveRight(StrafeMoveValue);
	}

	// Jump
	if (!GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.9f))
	{
		StartJump();
	}
	else if (GetCharacterMovement()->IsFalling()
		&& (FMath::FRandRange(0.0f, 1.0f) > 0.99f))
	{
		EndJump();
	}

	// Turning move
	ToHeadingRight = (GetActorRightVector() * Flat).GetSafeNormal();
	DotToTargetRight = FVector::DotProduct(ToHeadingRight, ToTargetNorm);
	if (FMath::Abs(DotToTargetRight) > 1.0f)
	{
		MoveTurnValue = FMath::Clamp(DotToTargetRight * 100.0f, -1.0f, 1.0f);
		
		BotMoveValueTurn = MoveTurnValue;
		///MoveTurn(MoveTurnValue);
		LastMoveLateral = MoveTurnValue;
	}
}

void AMechCharacter::BotMove()
{
	MoveForward(BotMoveValueForward);
	MoveRight(BotMoveValueStrafe);
	MoveTurn(BotMoveValueTurn);
}

void AMechCharacter::UpdateBotAim(float DeltaTime)
{
	FVector		TargetAimLocation = TargetMech->GetActorLocation();
	float		Distance = FVector::Dist(GetActorLocation(), TargetAimLocation);
	float		LeadFactor = 0.0f;
	if (GetEquippedTechActor() != nullptr)
	{
		LeadFactor = GetEquippedTechActor()->GetAimAheadFactor();
	}

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
	FVector MyVelocity = GetCharacterMovement()->Velocity * 0.1f;
	TargetAimLocation -= (LeadFactor * MyVelocity);

	
	// Lllline em up
	FVector AimToTarget = (TargetAimLocation - GetActorLocation());
	FVector ToPlayerSpeed = (AimToTarget + (PlayerVelocity * LeadFactor)).GetSafeNormal();
	FVector ToPlayerSpeedNorm = ToPlayerSpeed.GetSafeNormal();


	// Lateral mouse input
	FVector LateralAim = AimComponent->GetRightVector().GetSafeNormal();
	float LateralDot = FVector::DotProduct(LateralAim, ToPlayerSpeedNorm);
	float LateralInput = FMath::Clamp(LateralDot * 10.0f, -50.0f, 50.0f);
	BotMouseX = FMath::FInterpConstantTo(BotMouseX, LateralInput, DeltaTime, CameraSensitivity * 50.0f);

	// Vertical mouse input
	FVector VerticalAim = AimComponent->GetUpVector().GetSafeNormal();
	float VerticalDot = FVector::DotProduct(VerticalAim, ToPlayerSpeedNorm);
	float VerticalInput = FMath::Clamp((VerticalDot * 10.0f), -50.0f, 50.0f);
	BotMouseY = FMath::FInterpConstantTo(BotMouseY, VerticalInput, DeltaTime, CameraSensitivity * 50.0f);


	UpdateAim(DeltaTime);
}

float AMechCharacter::GetAngleToTarget()
{
	float Result = 0.0f;
	if (GetEquippedTechActor() != nullptr)
	{
		FVector TechVector = GetEquippedTechActor()->GetActorForwardVector().GetSafeNormal();
		FVector AimVector = (GetLookVector() - GetEquippedTechActor()->GetActorLocation()).GetSafeNormal();
		float DotToPerfectShot = FVector::DotProduct(TechVector, AimVector);
		Result = FMath::RadiansToDegrees(FMath::Acos(DotToPerfectShot));
	}
	return Result;
}

bool AMechCharacter::HasLineOfSightTo(FVector Location)
{
	bool Result = false;

	if (TargetMech != nullptr)
	{
		// First check for team safety
		bool bAimingAtTeammate = false;
		if (GetEquippedTechActor() != nullptr)
		{
			bAimingAtTeammate = GetEquippedTechActor()->GetTeamSafetyOn();
		}
		if (!bAimingAtTeammate)
		{

			// Otherwise go by direct linecast
			FHitResult Hit;
			FVector LineStart = TorsoCollider->GetComponentLocation() + (AimComponent->GetForwardVector() * 500.0f);
			FVector LineEnd = Location;
			bool Linecast = GetWorld()->LineTraceSingleByChannel(
				Hit,
				LineStart,
				LineEnd,
				ECollisionChannel::ECC_Pawn);
			if (Linecast)
			{
				if (Hit.Actor == TargetMech)
				{
					Result = true;
				}
			}
		}
	}

	return Result;
}

void AMechCharacter::SetTeam(int NewTeamID)
{
	TeamID = NewTeamID;
}


// Print to screen
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("hello"));
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Value: %f"), Value));

