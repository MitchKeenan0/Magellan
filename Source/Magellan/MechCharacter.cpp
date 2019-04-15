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
		FVector LastVelocity = GetCharacterMovement()->Velocity;
		LaunchCharacter(LastVelocity, true, true);

		if (Outfit != nullptr)
		{
			Outfit->ClearOutfit();
		}

		if (bCPU)
		{
			StopBotUpdate();
			SetLifeSpan(3.0f);
			GetController()->Destroy();
		}

		TorsoCollider->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TorsoCollider->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		TorsoCollider->SetSimulatePhysics(true);
		TorsoCollider->WakeRigidBody();
		
		GetMesh()->AddRelativeLocation(FVector(0.0f, 0.0f, -100.0f));
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		LegCollider->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
		
		FVector Offset = (FMath::VRand() * 100.0f);
		if (Offset.Z < 0.0f)
		{
			Offset.Z *= -1.0f;
		}
		FVector PopLocation = GetActorLocation() + Offset;
		TorsoCollider->AddImpulse((LastVelocity * 300.0f) + (Offset * 2500.0f));
		TorsoCollider->AddTorqueInRadians(Offset * 100.0f);
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
	GetWorld()->GetTimerManager().SetTimer(BotUpdateTimer, this, &AMechCharacter::UpdateBot, 0.01f, true, 0.2f);
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
		if (LegsAngle < -5.0f)
		{
			MoveTurn(AlignSpeed);
		}
		else if (LegsAngle > 5.0f)
		{
			MoveTurn(-AlignSpeed);
		}
	}
}
void AMechCharacter::MoveTurn(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar * 0.0001f);
	
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += (Value * GetWorld()->DeltaTimeSeconds * TurnSpeed);

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
	
	// Aim rotation
	FRotator MRotator = FRotator(Y, X, 0.0f) * CameraSensitivity;
	AimComponent->AddRelativeRotation(MRotator);
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -50.0f, 80.0f);
	AimComponent->SetRelativeRotation(AimRotation);

	// Torso rotation
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
			float Velocity = GetCharacterMovement()->Velocity.Size() * 0.04f;
			float Altitude = GetAltitude();
			bool bAirborne = (Altitude > 5.0f); /// GetCharacterMovement()->IsFalling();
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
	if (bCPU)
	{
		TargetMech = nullptr;
	}
	else if (OnHitDelegate.IsBound())
	{
		OnHitDelegate.Broadcast();
	}
}

void AMechCharacter::UpdateBot()
{
	if ((TargetMech != nullptr) && (TargetMech->GetLifeSpan() == 0.0f))
	{
		float DeltaTime = GetWorld()->DeltaTimeSeconds;
		UpdateBotAim(DeltaTime);

		UpdateBotMovement();

		if (GetAngleToTarget() < 5.0f)
		{
			if ((!bBotTriggerDown) && (FMath::FRandRange(0.0f, 1.0f) > 0.98f))
			{
				FVector TargetLocation = TargetMech->GetActorLocation();
				if (HasLineOfSightTo(TargetLocation))
				{
					PrimaryFire();
					bBotTriggerDown = true;
				}
			}
		}
		else if (bBotTriggerDown)
		{
			if (FMath::FRandRange(0.0f, 1.0f) > 0.9f)
			{
				bBotTriggerDown = false;
				PrimaryStopFire();
			}
		}
	}
	else
	{
		// Player
		//TargetMech = Cast<AMechCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

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
				if ((MechC != this) && (MechC->GetLifeSpan() == 0.0f))
				{
					// Check team
					if (MechC->GetTeam() != TeamID)
					{
						TargetMech = MechC;
					}
				}
			}

			/*for (AActor* Mech : Mechs)
			{

			}*/
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
	if (ToTarget.Size() >= 7000.0f)
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
	FVector		TargetLocation = TargetMech->GetActorLocation();
	float		Distance = FVector::Dist(GetActorLocation(), TargetLocation);

	// Above for gravity
	float DistSqr = FMath::Square(Distance);
	float UnitScale = 160000.0f;
	float ValueByDistance = DistSqr * 0.1f;
	float ExtraForSure = FMath::Sqrt(Distance);
	float VerticalAddition = (ValueByDistance + ExtraForSure) / UnitScale;
	TargetLocation.Z += VerticalAddition - 100.0f;

	// Ahead for velocity
	FVector PlayerVelocity = TargetMech->GetCharacterMovement()->Velocity * 0.3f;
	float TempScalar = 0.1f * FMath::Sqrt(Distance);
	TargetLocation += PlayerVelocity * TempScalar * 0.1f;

	// Local offset
	FVector Locality = 0.618f * (GetEquippedTechActor()->GetActorLocation() - GetActorLocation());
	TargetLocation -= Locality;

	
	// Lllline em up
	FVector ToTarget = (TargetLocation - GetActorLocation());
	FVector ToPlayerSpeed = (ToTarget + PlayerVelocity).GetSafeNormal();
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
		FHitResult Hit;
		FVector LineStart = TorsoCollider->GetComponentLocation() + (AimComponent->GetForwardVector() * 110.0f);
		bool Linecast = GetWorld()->LineTraceSingleByChannel(
			Hit, 
			LineStart,
			Location,
			ECollisionChannel::ECC_Pawn);
		if (Linecast)
		{
			if (Hit.Actor == TargetMech)
			{
				Result = true;
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

