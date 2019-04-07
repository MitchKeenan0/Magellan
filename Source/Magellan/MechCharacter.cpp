// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacter.h"
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

	JumpMaxHoldTime = MaxJumpTime;

	bUseControllerRotationYaw = false;

	//OnBrakeDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	//OnDodgeDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
	//OnLiftDelegate.AddDynamic(this, &AMechCharacter::TestFunction);
}

void AMechCharacter::TestFunction(bool bOn)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Delegate Received"));
}

// Called when the game starts or when spawned
void AMechCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = TopSpeed;

	EquipSelection(-1.0f);
}

void AMechCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void AMechCharacter::DestructMech()
{
	if (GetController() != nullptr)
	{
		GetController()->UnPossess();
	}

	if (bCPU)
	{
		StopBotUpdate();
		SetActorEnableCollision(false);
		SetLifeSpan(3.0f);
	}
	
	if (Outfit != nullptr)
	{
		Outfit->ClearOutfit();
	}

	TorsoCollider->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	TorsoCollider->SetSimulatePhysics(true);
	TorsoCollider->WakeRigidBody();

	FVector PopLocation = GetActorLocation() + (FMath::VRand() * 20.0f);
	TorsoCollider->AddRadialForce(PopLocation, 1500.0f, 11555000.0f, ERadialImpulseFalloff::RIF_Linear, false);
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
	}
	
	GetTorso()->SetOwnerNoSee(true);
	GetMesh()->SetOwnerNoSee(true);

	TrimOutfit();
	OffsetCamera(FVector::ZeroVector, FRotator::ZeroRotator, PlayerFOV);
	GetController()->SetControlRotation(GetActorRotation());

	FOutputDraw IdleDraw;
	OutputDraws.Init(IdleDraw, 1);
}

void AMechCharacter::StartBotUpdate()
{
	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.01f, true, 2.0f);
}
void AMechCharacter::StopBotUpdate()
{
	GetWorld()->GetTimerManager().ClearTimer(BotUpdateTimer);
}

// Called every frame
void AMechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ((Controller != nullptr) && (Controller->IsLocalController()))
	{
		UpdateLean(DeltaTime);
		UpdateTorso(DeltaTime);
		
		if (!bCPU)
		{
			UpdateTelemetry(DeltaTime);
		}
		/*else /// replaced with timer
		{
			UpdateBot(DeltaTime);
		}*/
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
	PlayerInputComponent->BindAction("Centre", IE_Pressed, this, &AMechCharacter::CentreMech);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AMechCharacter::Dodge);

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
}
void AMechCharacter::MoveTurn(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar * 0.0001f);
	
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += (Value * GetWorld()->DeltaTimeSeconds * TurnSpeed);

	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, GetWorld()->DeltaTimeSeconds, TurnSpeed);
	SetActorRotation(InterpRotation);

	if (LastMoveLateral == 0.0f)
	{
		/// also for lean
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

void AMechCharacter::UpdateTorso(float DeltaTime)
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
	
	// Set and clamp Aim
	FRotator MRotator = FRotator(Y, X, 0.0f) * CameraSensitivity;
	AimComponent->AddRelativeRotation(MRotator);
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -50.0f, 80.0f);
	AimComponent->SetRelativeRotation(AimRotation);

	// Interp rotation for torso
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
	SetActorRotation(InterpLean);
}

void AMechCharacter::UpdateTelemetry(float DeltaTime)
{
	if (TelemetryTimer >= (1.0f / TelemetryUpdateRate))
	{
		TelemetryTimer = 0.0f;

		if (OnTelemetryDelegate.IsBound())
		{
			bool bAirborne = GetCharacterMovement()->IsFalling();
			float Velocity = GetCharacterMovement()->Velocity.Size() * 0.04f;
			float Altitude = GetAltitude();
			OnTelemetryDelegate.Broadcast(bAirborne, Velocity, Altitude);
		}
	}
	else
	{
		TelemetryTimer += DeltaTime;
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
		Result = FVector::Dist(GetActorLocation(), Hit.ImpactPoint);
		Result *= 0.02f;
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

	if (HitResult && !(Hit.Actor->ActorHasTag("Ammo")))
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

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int i = 0; i < numTechs; ++i)
		{
			ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[i], SpawnInfo);
			if (NewTech != nullptr)
			{
				AvailableTechPointers.Add(NewTech);
				BuildTech(i, i);
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
			if (ThisTech->ActorHasTag(Tag))
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
	if (OnHitDelegate.IsBound())
	{
		OnHitDelegate.Broadcast();
	}
}

void AMechCharacter::UpdateBot()
{
	if (!TargetMech)
	{
		TargetMech = Cast<AMechCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
	else
	{
		float DeltaTime = GetWorld()->DeltaTimeSeconds;
		
		UpdateBotAim(DeltaTime);
		UpdateBotMovement();

		if (GetAngleToTarget() < 5.0f)
		{
			if ((!bBotTriggerDown) && (FMath::FRandRange(0.0f, 1.0f) > 0.9f))
			{
				PrimaryFire();
				bBotTriggerDown = true;
			}
		}
		else if (bBotTriggerDown)
		{
			if (FMath::FRandRange(0.0f, 1.0f) > 0.5f)
			{
				bBotTriggerDown = false;
				PrimaryStopFire();
			}
		}
	}
}

void AMechCharacter::UpdateBotMovement()
{
	// Rotation ingredients yummy
	FVector Flat = FVector(1.0f, 1.0f, 0.0f);
	FVector TargetLocation = TargetMech->GetActorLocation(); /// needs better "target location"
	FVector ToTarget = (TargetLocation - GetActorLocation());
	FVector ToTargetNorm = (ToTarget * Flat).GetSafeNormal();
	
	// Forward move
	if (ToTarget.Size() >= 10000.0f)
	{
		float ForwardMoveValue = FMath::Clamp(ToTarget.Size(), -1.0f, 1.0f);
		if (FMath::Abs(ForwardMoveValue) > 0.25f)
		{
			MoveForward(ForwardMoveValue);
		}
	}

	// Turning move
	FVector ToHeadingRight = (GetActorRightVector() * Flat).GetSafeNormal();
	float DotToTargetRight = FVector::DotProduct(ToHeadingRight, ToTargetNorm);
	if (FMath::Abs(DotToTargetRight) > 0.05f)
	{
		float MoveTurnValue = FMath::Clamp(DotToTargetRight * 100.0f, -1.0f, 1.0f);
		MoveTurn(MoveTurnValue);
		LastMoveLateral = MoveTurnValue;
	}
}

void AMechCharacter::UpdateBotAim(float DeltaTime)
{
	FVector ToPlayer = (TargetMech->GetActorLocation() - GetActorLocation());
	FVector PlayerVelocity = TargetMech->GetCharacterMovement()->Velocity * 0.1f;
	FVector ToPlayerSpeed = (ToPlayer + PlayerVelocity).GetSafeNormal();
	FVector ToPlayerSpeedNorm = ToPlayerSpeed.GetSafeNormal();
	FRotator MechLean = GetActorRotation();
	
	// Lateral
	FVector LateralAim = AimComponent->GetRightVector().GetSafeNormal();
	float LateralDot = FVector::DotProduct(LateralAim, ToPlayerSpeedNorm);
	float LateralInput = FMath::Clamp(LateralDot * 100.0f, -50.0f, 50.0f);
	BotMouseX = FMath::FInterpTo(BotMouseX, LateralInput, DeltaTime, CameraSensitivity * LateralDot);
	

	// Vertical
	FVector VerticalAim = AimComponent->GetUpVector().GetSafeNormal();
	float VerticalDot = FVector::DotProduct(VerticalAim, ToPlayerSpeedNorm);
	float PitchCorrection = -MechLean.Pitch * 0.2f;
	float VerticalInput = FMath::Clamp((VerticalDot * 100.0f) + PitchCorrection, -50.0f, 50.0f);
	BotMouseY = FMath::FInterpTo(BotMouseY, VerticalInput, DeltaTime, CameraSensitivity * VerticalDot);
}

float AMechCharacter::GetAngleToTarget()
{
	float Result = 0.0f;
	FVector TechVector = GetEquippedTechActor()->GetActorForwardVector().GetSafeNormal();
	FVector AimVector = (GetLookVector() - GetEquippedTechActor()->GetActorLocation()).GetSafeNormal();
	float DotToPerfectShot = FVector::DotProduct(TechVector, AimVector);
	Result = FMath::RadiansToDegrees(FMath::Acos(DotToPerfectShot));
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, FString::Printf(TEXT("TechAngleToTarget: %f"), Result));
	return Result;
}


// Print to screen
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("hello"));
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Value: %f"), Value));

