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

	GroundParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GroundParticles"));
	GroundParticles->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

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

void AMechCharacter::BotPrimaryTrigger(bool FireState)
{
	if (FireState)
	{
		PrimaryFire();
	} else
	{
		PrimaryStopFire();
	}
}

void AMechCharacter::BotSecondaryTrigger(bool FireState)
{
	if (FireState)
	{
		SecondaryFire();
	}
	else
	{
		SecondaryStopFire();
	}
}

void AMechCharacter::BotJumpTrigger(bool Value)
{
	if (Value)
	{
		StartJump();
	}
	else
	{
		EndJump();
	}
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
				MyPlayerController->SetInputMode(FInputModeGameAndUI());
				MyPlayerController->bShowMouseCursor = true;
				GetWorld()->GetGameViewport()->SetMouseLockMode(EMouseLockMode::DoNotLock);

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
	
	if (!bThirdPerson)
	{
		Torso->SetOwnerNoSee(true);
		GetMesh()->SetOwnerNoSee(true);

		OffsetCamera(FVector::ZeroVector, FRotator::ZeroRotator, PlayerFOV);
	}
	else if (!bCPU)
	{
		SpringArmComp->TargetArmLength = ThirdPersonDistance;
		SpringArmComp->SetRelativeLocation(ThirdPersonOffset);

		OffsetCamera(ThirdPersonOffset, FRotator::ZeroRotator, PlayerFOV);
	}

	InitOptions();
	TrimOutfit();

	GetController()->SetControlRotation(GetActorRotation());

	FOutputDraw IdleDraw;
	OutputDraws.Init(IdleDraw, 1);
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
			UpdateScope();

			TelemetryTimer += DeltaTime;
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

// Input
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

void AMechCharacter::MoveRight(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar);

	// Conditional leg-align
	if (((Value < 0.0f) && (GetLegsToTorsoAngle() > 0.0f))
		|| ((Value > 0.0f) && (GetLegsToTorsoAngle() < 0.0f)))
	{
		MoveTurn(Value * 0.15f);
	}

	/// for dodge direction
	LastMoveLateral = Value;
}

void AMechCharacter::MoveForward(float Value)
{
	AddMovementInput(GetMesh()->GetForwardVector(), Value * MoveSpeed);

	LastMoveForward = Value;

	// Bring legs around towards look direction
	if (Value > 0.0f)
	{
		float LegsAngle = GetLegsToTorsoAngle();
		float AlignAngleScale = FMath::Clamp(LegsAngle * 0.1f, -1.0f, 1.0f);
		float VelAlignScale = FMath::Clamp(GetVelocity().Size(), 0.1f, 1000.0f) * 0.0015f;
		
		float AlignSpeed = FMath::Abs(VelAlignScale * AlignAngleScale) * 0.015f * TorsoSpeed;
		
		if (LegsAngle < -0.0f)
		{
			MoveTurn(AlignSpeed);
		}
		else if (LegsAngle > 0.0f)
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
	if (!bDead)
	{
		bScoping = true;
	}
}

void AMechCharacter::EndScope()
{
	bScoping = false;
}

void AMechCharacter::UpdateScope()
{
	float CurrentFOV = CameraComp->FieldOfView;
	float Vignette = CameraComp->PostProcessSettings.VignetteIntensity;

	if (bScoping)
	{
		FVector ScopePosition = AimComponent->GetRelativeTransform().GetLocation().ForwardVector * 1000.0f; /// to do change this
		
		float InterpFOV = FMath::FInterpTo(CurrentFOV, ScopeFOV, GetWorld()->DeltaTimeSeconds, 15.0f);
		InterpFOV = FMath::Clamp(InterpFOV, ScopeFOV, PlayerFOV);
		OffsetCamera(ScopePosition, FRotator::ZeroRotator, InterpFOV);

		float InterpVignette = FMath::FInterpTo(Vignette, 2.0f, GetWorld()->DeltaTimeSeconds, 15.0f);
		CameraComp->PostProcessSettings.VignetteIntensity = InterpVignette;
	}
	else if (CurrentFOV != PlayerFOV)
	{
		if (bThirdPerson)
		{
			OffsetCamera(ThirdPersonOffset, FRotator::ZeroRotator, PlayerFOV);
		}
		else
		{
			OffsetCamera(FVector::ZeroVector, FRotator::ZeroRotator, PlayerFOV);
		}

		CameraComp->PostProcessSettings.VignetteIntensity = 0.0f;
	}
}

// Equip select & Tech actor
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

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputMouseDelta(X, Y);

	// Aim rotation
	FRotator MRotator = FRotator(Y, X, 0.0f) * CameraSensitivity;
	AimComponent->AddRelativeRotation(MRotator);
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	FRotator Current = AimComponent->GetRelativeTransform().Rotator();
	AimRotation = FMath::RInterpTo(Current, AimRotation, DeltaTime, CameraSensitivity * 10.0f);
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -70.0f, 80.0f);
	AimRotation.Roll = GetActorRotation().Roll * -0.5f;
	
	AimComponent->SetRelativeRotation(AimRotation);
}

void AMechCharacter::BotAimTo(FRotator AimRotation)
{
	AimComponent->SetRelativeRotation(AimRotation);

	GetAltitude();
}

void AMechCharacter::UpdateTorso(float DeltaTime)
{
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	FRotator CurrentR = TorsoCollider->GetRelativeTransform().Rotator();
	FRotator InterpRotator = FMath::RInterpTo(CurrentR, AimRotation, DeltaTime, TorsoSpeed);
	InterpRotator.Pitch = FMath::Clamp(InterpRotator.Pitch, TorsoMinPitch, TorsoMaxPitch);
	InterpRotator.Roll *= -0.8f;
	
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
	float DotToVelocityRight = FVector::DotProduct(MyVelocity, MyRight) + (LastMoveLateral * 0.25f);
	if (DotToVelocityRight == 0.0f)
	{
		float SideVelocity = FMath::Clamp(GetCharacterMovement()->Velocity.ForwardVector.Y * 0.001f, -1.0f, 1.0f);
		DotToVelocityRight = SideVelocity;
	}

	// Initial rotation
	Lean.Roll = DotToVelocityRight * MoveTilt; ///LastMoveLateral * MoveTilt;
	Lean.Pitch = DotToVelocityForward * -MoveTilt;
	Lean.Yaw = GetActorRotation().Yaw;
	
	// Velocity mapped 0.0 -- 1.0
	float a1 = 1.0f;
	float a2 = TopSpeed;
	float s = GetCharacterMovement()->Velocity.Size();
	float b1 = 0.01f;
	float b2 = 1.0f;
	float t = b1 + (((s - a1) * (b2 - b1)) / (a2 - a1));
	float ScaledVelocity = t; ///  FMath::Sqrt(t);

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
	FRotator InterpLean = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, TurnSpeed);

	FRotator DeltaRotation = InterpLean - GetActorRotation();
	DeltaRotation.Roll = (DeltaRotation.Roll * -0.0314f);

	SetActorRotation(InterpLean);
	AimComponent->AddRelativeRotation(DeltaRotation * -0.8f);
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
	FVector Start = GetActorLocation(); /// from the feet
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
		float Distance = (FVector::Dist(GetActorLocation(), Hit.ImpactPoint) * 0.02f) - 4; /// terrible hardcode!
		
		Result = Distance;

		// Airborne
		if (FMath::Abs(Distance) > 3.0f)
		{
			GroundParticles->SetVisibility(false);
		}

		// Grounded
		else if (GroundParticles != nullptr)
		{
			GroundParticles->SetWorldRotation(Hit.ImpactNormal.Rotation());

			GroundParticles->SetVisibility(true);
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
	FVector RaycastVector = CameraComp->GetForwardVector() * 50000.0f; ///AimComponent->GetForwardVector() * 50000.0f;
	FVector Start = CameraComp->GetComponentLocation(); ///AimComponent->GetComponentLocation();
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

	/*if (bThirdPerson)
	{
		Result += ThirdPersonOffset;
	}*/

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

		if (!bCPU && (OnTargetScanDelegate.IsBound()))
		{
			OnTargetScanDelegate.Broadcast(true);
		}
	}
}

void AMechCharacter::SecondaryStopFire()
{
	if (TargetingComputer != nullptr)
	{
		TargetingComputer->DeactivateTechComponent();

		if (!bCPU && (OnTargetScanDelegate.IsBound()))
		{
			OnTargetScanDelegate.Broadcast(false);
		}
	}
}

void AMechCharacter::BuildTech(int TechID, int TechHardpoint)
{
	if (AvailableTechPointers.IsValidIndex(TechID)) ///  && (AvailableTechPointers.Num() >= (TechID + 1))
	{
		ATechActor* NewTechType = AvailableTechPointers[TechID];
		if (NewTechType != nullptr)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[TechID], SpawnInfo);
			if (NewTech != nullptr)
			{
				// Attach to mech
				if (Outfit->HardpointTechs[TechHardpoint] != nullptr)
				{
					Outfit->HardpointTechs[TechHardpoint]->Destroy();
					Outfit->HardpointTechs.RemoveAt(TechHardpoint);
				}
				
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

void AMechCharacter::SaveChoice(int TechID, int HardpointIndex)
{
	UMechSaveGame* SaveGameInstance = Cast<UMechSaveGame>(UGameplayStatics::CreateSaveGameObject(UMechSaveGame::StaticClass()));

	SaveGameInstance->Hardpoints[HardpointIndex] = TechID;
	SaveGameInstance->bChanged = true;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, FString::Printf(TEXT("Saved tech %i in slot %i"), TechID, HardpointIndex));
}

void AMechCharacter::InitOptions()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Initalize loadout
	if (Outfit->HardpointTechs.Num() == 0)
	{
		int numAvailable = AvailableTech.Num();
		if (numAvailable > 0)
		{
			Outfit->HardpointTechs.Init(nullptr, numAvailable);

			for (int i = 0; i != numAvailable; ++i)
			{
				if (AvailableTech[i] != nullptr)
				{

					ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[i], SpawnInfo);
					if (NewTech != nullptr)
					{
						AvailableTechPointers.Insert(NewTech, i);

						if (i < 2)
						{
							BuildTech(i, i);
						}
					}
				}
			}
		}
	}
	
	// Load mech data
	UMechSaveGame* LoadGameInstance = Cast<UMechSaveGame>(UGameplayStatics::CreateSaveGameObject(UMechSaveGame::StaticClass()));
	LoadGameInstance = Cast<UMechSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveSlotName, LoadGameInstance->UserIndex));
	if ((LoadGameInstance != nullptr)
		&& (LoadGameInstance->bChanged == true))
	{
		// Update loadout
		TArray<int32> SavedHardpoints = LoadGameInstance->Hardpoints;
		int NumHardpoints = SavedHardpoints.Num();

		for (int32 Index = 0; Index != NumHardpoints; ++Index)
		{
			int Choice = SavedHardpoints[Index];

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("Reading tech %i in slot %i"), Choice, Index));

			if (AvailableTech.IsValidIndex(Choice) && (AvailableTech[Choice] != nullptr))
			{
				ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[Choice], SpawnInfo);
				if (NewTech != nullptr)
				{
					if (Index < NumHardpoints)
					{
						if (Outfit->HardpointTechs.IsValidIndex(Index))
						{
							RemovePart(0, Index);
						}

						BuildTech(Choice, Index);
					}
				}
			}
		}
	}

	// Targeting computer
	int NumTargeters = TargetingTech.Num();
	if (NumTargeters > 0)
	{
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

	// Output
	///
}

TArray<ATechActor*> AMechCharacter::GetBuilderTechByTag(FName Tag)
{
	TArray<ATechActor*> Result;
	
	int numTechPtrs = AvailableTechPointers.Num();
	if (numTechPtrs > 0)
	{
		for (int i = 0; i != numTechPtrs; ++i)
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
		for (int i = 0; i != NumTechs; ++i)
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
	if (Outfit->HardpointTechs.IsValidIndex(HardpointIndex) && (NumTechs >= (HardpointIndex + 1)))
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

	if (!bCPU && (CameraShakeOnDamage != nullptr))
	{
		APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (MyController != nullptr)
		{
			MyController->ClientPlayCameraShake(CameraShakeOnDamage, Damage, ECameraAnimPlaySpace::CameraLocal, GetActorRotation());

			if (OnDamageDelegate.IsBound())
			{
				OnDamageDelegate.Broadcast(Damage);
			}
		}
	}

	///GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("OUCH"));

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AMechCharacter::ReceiveLock()
{
	
	if (!bCPU && OnReceiveLockDelegate.IsBound())
	{
		OnReceiveLockDelegate.Broadcast();
	}
}

void AMechCharacter::UpdateTargets()
{
	if (TargetingComputer != nullptr)
	{
		LockedTargets = TargetingComputer->GetLockedTargets();
		
		// For bot
		if ((LockedTargets.Num() > 0) && (LockedTargets[0] != nullptr) && (TargetMech == nullptr))
		{
			TargetMech = Cast<AMechCharacter>(LockedTargets[0]);
		}
		
		// Hud update
		if (!bDead && !bCPU && OnTargetLockDelegate.IsBound())
		{
			OnTargetLockDelegate.Broadcast(true);
		}

		if ((LockedTargets.Num() == 0) && (TargetMech != nullptr))
		{
			TargetMech = nullptr;
			PrimaryStopFire();
			SecondaryStopFire();
		}
	}
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
			ECollisionChannel::ECC_Visibility);
		
		if (!Linecast)
		{
			Result = true;
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

