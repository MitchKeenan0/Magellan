// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacter.h"

// Sets default values
AMechCharacter::AMechCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AimComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AimComponent"));
	AimComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	Torso->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Outfit = CreateDefaultSubobject<UMechOutfitComponent>(TEXT("Outfit"));
	Outfit->RegisterComponent();

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
}

// Called when the game starts or when spawned
void AMechCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//BuildTech(0);
}

// Called every frame
void AMechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ((Controller != nullptr) && (Controller->IsLocalController()))
	{
		UpdateTorso(DeltaTime);
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

	// Axes
	PlayerInputComponent->BindAxis("MoveRight", this, &AMechCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMechCharacter::MoveForward);
	PlayerInputComponent->BindAxis("EquipSelect", this, &AMechCharacter::EquipSelection);
}

// Movement
void AMechCharacter::MoveRight(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar);

	// & Turning
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += (Value * GetWorld()->DeltaTimeSeconds * TurnSpeed);
	SetActorRotation(NewRotation);
}
void AMechCharacter::MoveForward(float Value)
{
	AddMovementInput(GetMesh()->GetForwardVector(), Value * MoveSpeed);
}

// Jump
void AMechCharacter::StartJump()
{
	Jump();
}
void AMechCharacter::EndJump()
{
	StopJumping();
}

// Brake
void AMechCharacter::StartBrake()
{
	GetCharacterMovement()->MaxWalkSpeed = BrakeStrength;
	if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->BrakingDecelerationFalling = GetCharacterMovement()->Velocity.Size();
		GetCharacterMovement()->GravityScale = 0.3f;
	}
	bBraking = true;
}

void AMechCharacter::EndBrake()
{
	GetCharacterMovement()->MaxWalkSpeed = 7200.0f;
	if (GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->BrakingDecelerationFalling = 50.0f;
		GetCharacterMovement()->GravityScale = 1.0f;
	}
	bBraking = false;
}

void AMechCharacter::CentreMech()
{
	if (Torso != nullptr)
	{
		Torso->SetRelativeRotation(FRotator::ZeroRotator);
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
					ThisTech->SetActorRotation(Torso->GetComponentRotation());
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

void AMechCharacter::UpdateTorso(float DeltaTime)
{
	// Get Mouse inputs
	float X;
	float Y;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputMouseDelta(X, Y);
	
	// Set and clamp Aim
	FRotator MRotator = FRotator(Y, X, 0.0f);
	AimComponent->AddRelativeRotation(MRotator);
	FRotator AimRotation = AimComponent->GetRelativeTransform().Rotator();
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -25.0f, 80.0f);
	AimComponent->SetRelativeRotation(AimRotation);

	// Interp rotation for torso
	FRotator CurrentR = Torso->GetRelativeTransform().Rotator();
	FRotator InterpRotator = FMath::RInterpTo(CurrentR, AimRotation, DeltaTime, TorsoSpeed);
	InterpRotator.Pitch = FMath::Clamp(InterpRotator.Pitch, TorsoMinPitch, TorsoMaxPitch);
	Torso->SetRelativeRotation(InterpRotator);
}

FVector AMechCharacter::GetLookVector()
{
	// Initial aim, in case ray hits nothing
	FVector Result = AimComponent->GetComponentLocation() + (AimComponent->GetForwardVector() * 10000.0f);

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
	IgnoredActors.Add(Torso->GetOwner());
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

	if (HitResult)
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
		ATechActor* MyPrimaryTech = Outfit->HardpointTechs[EquipSelectValue];
		if (MyPrimaryTech != nullptr)
		{
			Result = MyPrimaryTech->GetAimPoint();
		}
	}
	
	if (Result == FVector::ZeroVector)
	{
		Result = GetActorLocation() + (CameraComp->GetForwardVector() * 100.0f);
	}
	return Result;
}

void AMechCharacter::PrimaryFire()
{
	if (Outfit->HardpointTechs.Num() >= (EquipSelectValue + 1))
	{
		ATechActor* MyPrimaryTech = Outfit->HardpointTechs[EquipSelectValue];
		if (MyPrimaryTech != nullptr)
		{
			MyPrimaryTech->ActivateTech();
		}
	}
}

void AMechCharacter::PrimaryStopFire()
{
	if (Outfit->HardpointTechs.Num() >= (EquipSelectValue + 1))
	{
		ATechActor* MyPrimaryTech = Outfit->HardpointTechs[EquipSelectValue];
		if (MyPrimaryTech != nullptr)
		{
			MyPrimaryTech->DeactivateTech();
		}
	}
}

void AMechCharacter::BuildTech(int TechID, int TechHardpoint)
{
	if (AvailableTechPointers.Num() >= (TechID + 1))
	{
		ATechActor* NewTech = AvailableTechPointers[TechID];
		if (NewTech != nullptr)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[TechID], SpawnInfo);
			if (NewTech != nullptr)
			{
				Outfit->HardpointTechs.Insert(NewTech, TechHardpoint);
				
				///GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Fitted new tech"));

				NewTech->AttachToComponent(Torso, FAttachmentTransformRules::KeepWorldTransform);

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
				GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Spawned new tech"));
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
	CameraComp->FieldOfView = FOV;
	SpringArmComp->RelativeRotation = Rotation;
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
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Removed Tech"));
		}
	}
}


// Print to screen
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("hello"));
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Value: %f"), Value));

