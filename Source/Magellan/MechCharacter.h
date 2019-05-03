// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Magellan.h"
#include "GameFramework/Character.h"
#include "TechActor.h"
#include "PlayerIDWidgetComponent.h"
#include "MechOutfitComponent.h"
///#include "PaperSpriteComponent.h"
#include "MechCharacter.generated.h"

class UTargetingTechComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHitDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTargetLockDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReceiveLockDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBrakeDelegate, bool, bOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDodgeDelegate, bool, bOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLiftDelegate, bool, bOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTelemetryDelegate, bool, bAirborne, float, Velocity, float, Altitude);
///DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestDelegate, float, Damage);


USTRUCT()
struct FOutputDraw
{
	GENERATED_BODY()

	UPROPERTY()
	float DrawSize;

	UPROPERTY()
	float DrawHeat;

	void InitDraw(const float Size, const float Heat)
	{
		DrawSize = Size;
		DrawHeat = Heat;
	}

	void SetDraw(const float Value)
	{
		DrawSize = Value;
	}
	void SetHeat(const float Value)
	{
		DrawHeat = Value;
	}

	//Constructor
	FOutputDraw()
	{
		DrawSize = 1.0f;
		DrawHeat = 0.0f;
	}

	//For GC
	void Destroy()
	{

	}
};


UCLASS()
class MAGELLAN_API AMechCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY()
	float InternalMoveSpeed = 0.0f;

	void UpdateTorso(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	TArray<ATechActor*> GetBuilderTechByTag(FName Tag);


public:
	// Sets default values for this character's properties
	AMechCharacter();

	UFUNCTION(BlueprintCallable)
	void InitOptions();

	UFUNCTION(BlueprintCallable)
	void BuildTech(int TechID, int TechHardpoint);

	UFUNCTION(BlueprintCallable)
	void TrimOutfit();

	UFUNCTION(BlueprintCallable)
	void SetTeam(int NewTeamID);

	UFUNCTION(BlueprintCallable)
	int GetTeam() { return TeamID; }

	UFUNCTION(BlueprintCallable)
	void ReceiveLock();

	UFUNCTION()
	void ConfirmHit();

	UFUNCTION()
	void DestructMech();


	UFUNCTION(BlueprintCallable)
	FVector GetLookVector();

	UFUNCTION(BlueprintCallable)
	FVector GetAimPoint();

	UFUNCTION(BlueprintCallable)
	FVector GetTorsoPoint();

	UFUNCTION(BlueprintCallable)
	AMechCharacter* GetTargetMech() { return TargetMech; }

	UFUNCTION()
	bool HasLineOfSightTo(FVector Location);

	UFUNCTION(BlueprintCallable)
	UBoxComponent* GetTorso() { return TorsoCollider; }

	UFUNCTION()
	void SetIsBot(bool Value) { bCPU = Value; }

	UFUNCTION(BlueprintCallable)
	bool IsBot() { return bCPU; }

	UFUNCTION(BlueprintCallable)
	bool IsDead() { return bDead; }

	UFUNCTION()
	void StartBotUpdate();

	UFUNCTION()
	void StopBotUpdate();

	UFUNCTION()
	void UpdateBotMovement();

	UFUNCTION()
	void BotMove();

	UFUNCTION()
	void UpdateBotAim(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	FString GetMechName() { return MechName; }

	UFUNCTION(BlueprintCallable)
	void SetMechName(FString Value);


	UFUNCTION()
	void TestFunction(bool bOn);

	/*UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FTestDelegate OnTestDelegate;*/

	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FBrakeDelegate OnBrakeDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FDodgeDelegate OnDodgeDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FLiftDelegate OnLiftDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FTelemetryDelegate OnTelemetryDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FHitDelegate OnHitDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FTargetLockDelegate OnTargetLockDelegate;
	UPROPERTY(BlueprintAssignable, Category = "TestDelegate")
	FReceiveLockDelegate OnReceiveLockDelegate;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle BotUpdateTimer;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle PlayerUpdateTimer;

	/*UPROPERTY(BlueprintReadOnly)
	FTimerHandle BotUpdateTimer;*/

	UFUNCTION()
	void UpdateAim(float DeltaTime);

	UFUNCTION()
	void UpdatePlayer();

	UFUNCTION(BlueprintCallable)
	void UpdateBot();

	void MoveRight(float Value);
	void MoveForward(float Value);
	void MoveTurn(float Value);
	void StartJump();
	void EndJump();
	void StartBrake();
	void EndBrake();
	void Dodge();
	void CentreMech();
	void EquipSelection(float Value);
	void StartScope();
	void EndScope();

	void PrimaryFire();
	void PrimaryStopFire();
	void SecondaryFire();
	void SecondaryStopFire();

	float GetAngleToTarget();

	void UpdateLean(float DeltaTime);
	
	UFUNCTION()
	void UpdateTelemetry(float DeltaTime);

	UFUNCTION()
	float GetAltitude();

	UFUNCTION(BlueprintCallable)
	bool IsBraking() { return bBraking; }

	UFUNCTION(BlueprintCallable)
	FName GetEquippedTechName();

	UFUNCTION(BlueprintCallable)
	ATechActor* GetTechActor(int EquipIndex);

	UFUNCTION(BlueprintCallable)
	ATechActor* GetEquippedTechActor();

	UFUNCTION(BlueprintCallable)
	UMechOutfitComponent* GetOutfit() { return Outfit; } /// oh my god Beckyy

	UFUNCTION(BlueprintCallable)
	float GetLegsToTorsoAngle();

	UFUNCTION(BlueprintCallable)
	void RemovePart(int TechID, int HardpointIndex);

	UFUNCTION(BlueprintCallable)
	void OffsetCamera(FVector Offset, FRotator Rotation, float FOV);

	UFUNCTION(BlueprintCallable)
	void InitMech();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USceneComponent* AimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Body")
	UBoxComponent* TorsoCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Body")
	USkeletalMeshComponent* Torso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	UBoxComponent* LegCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	UMechOutfitComponent* Outfit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Name")
	UPlayerIDWidgetComponent* PlayerIDComp;
	

	// Reference UMG Asset in the Editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HudWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> TargeterWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> DeathWidgetClass;

	// Variable to hold the widget After Creating it.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* MyHud;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* MyTargeter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* MyDeathScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* DestructParticles;

	UPROPERTY(BlueprintReadOnly)
	UParticleSystemComponent* MyDestructParticles;



	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UPaperSpriteComponent* ControlPanelSprite;*/

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float PlayerUpdateRate = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxHealth = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float LateralMoveScalar = 0.001f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TopSpeed = 7200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MoveTilt = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float DodgeSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float BrakeStrength = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxJumpTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TurnSpeed = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoSpeed = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoMinPitch = -25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoMaxPitch = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float PlayerFOV = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float ScopeFOV = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float CameraSensitivity = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TelemetryUpdateRate = 30.0f;


	// Tech
	UPROPERTY(BlueprintReadWrite, Category = "Tech")
	ATechActor* EquippedTechActor = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ATechActor>> AvailableTech;

	UPROPERTY(EditDefaultsOnly)
	TArray<ATechActor*> AvailableTechPointers;

	UPROPERTY(BlueprintReadOnly)
	float EquipSelectValue = 0;


	// Targeting
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UTargetingTechComponent>> TargetingTech;

	UPROPERTY(BlueprintReadOnly)
	UTargetingTechComponent* TargetingComputer;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> LockedTargets;

	UPROPERTY()
	FTimerHandle TargetUpdateTimer;

	UFUNCTION()
	void UpdateTargets();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	UPROPERTY()
	float MyHealth = -1.0f;

	UPROPERTY()
	bool bCPU = false;

	UPROPERTY()
	bool bDead = false;

	UPROPERTY()
	bool bVisible = false;

	UPROPERTY()
	int TeamID = 0;

	UPROPERTY()
	float BotMoveValueForward = 0.0f;

	UPROPERTY()
	float BotMoveValueStrafe = 0.0f;

	UPROPERTY()
	float BotMoveValueTurn = 0.0f;

	UPROPERTY()
	bool bBotTriggerDown = false;

	UPROPERTY()
	float TimeAtTriggerDown = 0.0f;

	UPROPERTY()
	float TimeAtTriggerUp = 0.0f;

	UPROPERTY()
	float BotBurstDuration = 1.0f;

	UPROPERTY()
	bool bBraking = false;

	UPROPERTY()
	float LastMoveForward = 0.0f;

	UPROPERTY()
	float LastMoveLateral = 0.0f;

	UPROPERTY()
	float TelemetryTimer = 0.0f;

	UPROPERTY()
	float BotMouseX = 0.0f;

	UPROPERTY()
	float BotMouseY = 0.0f;

	UPROPERTY()
	AMechCharacter* TargetMech = nullptr;

	UPROPERTY()
	FString MechName = " ";

	UPROPERTY()
	TArray<FOutputDraw> OutputDraws;

	UPROPERTY()
	FVector Flat = FVector(1.0f, 1.0f, 0.0f);

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector ToTarget = FVector::ZeroVector;

	UPROPERTY()
	FVector ToTargetNorm = FVector::ZeroVector;

	UPROPERTY()
	FVector ToHeadingRight = FVector::ZeroVector;

	UPROPERTY()
	float ForwardMoveValue = 0.0f;

	UPROPERTY()
	float LateralBias = 0.0f;

	UPROPERTY()
	float StrafeMoveValue = 0.0f;

	UPROPERTY()
	float DotToTargetRight = 0.0f;

	UPROPERTY()
	float MoveTurnValue = 0.0f;

};
