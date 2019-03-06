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

	JumpMaxHoldTime = MaxJumpTime;
}

// Called when the game starts or when spawned
void AMechCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	BuildTech();
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
	AimRotation.Pitch = FMath::Clamp(AimRotation.Pitch, -15.0f, 80.0f);
	AimComponent->SetRelativeRotation(AimRotation);

	// Interp rotation for torso
	FRotator CurrentR = Torso->GetRelativeTransform().Rotator();
	FRotator InterpRotator = FMath::RInterpTo(CurrentR, AimRotation, DeltaTime, TorsoSpeed);
	InterpRotator.Pitch = FMath::Clamp(InterpRotator.Pitch, TorsoMinPitch, TorsoMaxPitch);
	Torso->SetRelativeRotation(InterpRotator);
}

void AMechCharacter::BuildTech()
{
	if (TechSubclass != nullptr)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ATechComponent* NewTech = GetWorld()->SpawnActor<ATechComponent>(TechSubclass, GetActorLocation(), GetActorRotation(), SpawnInfo);
		if (NewTech != nullptr)
		{
			NewTech->AttachToComponent(Torso, FAttachmentTransformRules::KeepWorldTransform);
			NewTech->SetActorRelativeLocation(FVector(-40.0f, 150.0f, 80.0f));
		}
	}
}


// Print to screen
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, TEXT("hello"));
// GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, FString::Printf(TEXT("Value: %f"), Value));

