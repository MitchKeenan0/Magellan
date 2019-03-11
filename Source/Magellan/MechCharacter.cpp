// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacter.h"

// Sets default values
AMechCharacter::AMechCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AimComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AimComponent"));
	AimComponent->AttachTo(RootComponent);

	Torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	Torso->AttachTo(RootComponent);

	Outfit = CreateDefaultSubobject<UMechOutfitComponent>(TEXT("Outfit"));

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->AttachTo(AimComponent);
	SpringArmComp->SetRelativeLocation(FVector(10.0f, 0.0f, -10.0f));
	SpringArmComp->TargetArmLength = -20.0f;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 10.0f;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraRotationLagSpeed = 8.0f;
	SpringArmComp->CameraLagMaxDistance = 15.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachTo(SpringArmComp);
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
	PlayerInputComponent->BindAction("PrimaryFire", IE_Pressed, this, &AMechCharacter::PrimaryFire);

	// Axes
	PlayerInputComponent->BindAxis("MoveRight", this, &AMechCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMechCharacter::MoveForward);
}

// Movement
void AMechCharacter::MoveRight(float Value)
{
	AddMovementInput(GetMesh()->GetRightVector(), Value * MoveSpeed * LateralMoveScalar);

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
	FVector Result = AimComponent->GetForwardVector();
	return Result;
}

void AMechCharacter::PrimaryFire()
{
	ATechActor* MyPrimaryTech = Outfit->HardpointTechs[0];
	if (MyPrimaryTech != nullptr)
	{
		MyPrimaryTech->ActivateTech();
	}
}

void AMechCharacter::BuildTech(int TechID, int TechHardpoint)
{
	ATechActor* NewTech = Outfit->HardpointTechs[TechID];
	if (NewTech != nullptr)
	{
		NewTech->AttachToComponent(Torso, FAttachmentTransformRules::KeepWorldTransform);

		FVector SetLocation = Outfit->HardpointLocations[TechHardpoint];
		NewTech->SetActorRelativeLocation(SetLocation);
		
		NewTech->SetActorRelativeRotation(FRotator::ZeroRotator);
	}
}

TArray<ATechActor*> AMechCharacter::GetBuilderTechByTag(FName Tag)
{
	TArray<ATechActor*> Result;
	
	int numTechs = AvailableTech.Num();
	int numTechPtrs = AvailableTechPointers.Num();
	
	// Init Tech
	if ((numTechPtrs == 0.0f) && (numTechs > 0))
	{
		for (int i = 0; i < numTechs; ++i)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ATechActor* NewTech = GetWorld()->SpawnActor<ATechActor>(AvailableTech[i], SpawnInfo);
			if (NewTech != nullptr)
			{
				if (NewTech->ActorHasTag(Tag))
				{
					NewTech->InitTechActor(this);
					AvailableTechPointers.Add(NewTech);
					Result.Add(NewTech);
					Outfit->HardpointTechs.Add(NewTech);
					GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("Spawned new tech"));
				}
				else
				{
					NewTech->Destroy();
				}
			}
		}
	}

	if (AvailableTechPointers.Num() > 0)
	{
		int nTechPtrs = AvailableTechPointers.Num();

		// Compare against existing tech
		for (int i = 0; i < nTechPtrs; ++i)
		{
			ATechActor* ThisTech = AvailableTechPointers[i];
			if (ThisTech->ActorHasTag(Tag)
				 && !(AvailableTechPointers.Contains(ThisTech))
				&& !(Result.Contains(ThisTech)))
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


// Print to screen
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("hello"));
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Value: %f"), Value));

