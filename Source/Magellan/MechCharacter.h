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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveRight(float Value);
	void MoveForward(float Value);
	void StartJump();
	void EndJump();

	UFUNCTION(BlueprintCallable)
	void BuildTech();

	UFUNCTION(BlueprintCallable)
	void OffsetCamera(FVector Offset, FRotator Rotation, float FOV);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USceneComponent* AimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USkeletalMeshComponent* Torso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	UMechOutfitComponent* Outfit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;



	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UPaperSpriteComponent* ControlPanelSprite;*/

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float LateralMoveScalar = 0.001f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxJumpTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TurnSpeed = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoSpeed = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoMaxPitch = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float TorsoMinPitch = -6.0f;


	// Tech
	UPROPERTY(EditDefaultsOnly, Category = "Tech")
	TSubclassOf<ATechActor> TechActorSubclass;

	UPROPERTY(BlueprintReadWrite, Category = "Tech")
	ATechActor* EquippedTechActor = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ATechActor>> AvailableTech;

	UPROPERTY(EditDefaultsOnly)
	TArray<ATechActor*> AvailableTechPointers;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
