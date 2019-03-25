// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Magellan.h"
#include "GameFramework/Character.h"
#include "TechActor.h"
#include "MechOutfitComponent.h"
//#include "PaperSpriteComponent.h"
#include "MechCharacter.generated.h"

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
	FVector GetLookVector();

	UFUNCTION(BlueprintCallable)
	FVector GetAimPoint();

	UFUNCTION(BlueprintCallable)
	FVector GetTorsoPoint();

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetTorso() {return Torso;}

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveRight(float Value);
	void MoveForward(float Value);
	void StartJump();
	void EndJump();
	void StartBrake();
	void EndBrake();
	void Dodge();
	void CentreMech();
	void EquipSelection(float Value);

	void PrimaryFire();
	void PrimaryStopFire();

	void UpdateLean(float DeltaTime);

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
	void InitOptions();

	UFUNCTION(BlueprintCallable)
	void BuildTech(int TechID, int TechHardpoint);

	UFUNCTION(BlueprintCallable)
	void TrimOutfit();

	UFUNCTION(BlueprintCallable)
	void RemovePart(int TechID, int HardpointIndex);

	UFUNCTION(BlueprintCallable)
	void OffsetCamera(FVector Offset, FRotator Rotation, float FOV);

	UFUNCTION(BlueprintCallable)
	void InitMech();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USceneComponent* AimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Body")
	USkeletalMeshComponent* Torso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	UMechOutfitComponent* Outfit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	// Reference UMG Asset in the Editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HudWidgetClass;

	// Variable to hold the widget After Creating it.
	UUserWidget* MyHud;



	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UPaperSpriteComponent* ControlPanelSprite;*/

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
	float FOV = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float CameraSensitivity = 1.0f;


	// Tech
	UPROPERTY(BlueprintReadWrite, Category = "Tech")
	ATechActor* EquippedTechActor = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ATechActor>> AvailableTech;

	UPROPERTY(EditDefaultsOnly)
	TArray<ATechActor*> AvailableTechPointers;

	UPROPERTY(BlueprintReadOnly)
	float EquipSelectValue = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	UPROPERTY()
	bool bBraking;

	UPROPERTY()
	float LastMoveForward = 0.0f;

	UPROPERTY()
	float LastMoveLateral = 0.0f;

};
