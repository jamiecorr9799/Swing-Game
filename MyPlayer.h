// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TriggerVolume.h"
#include "MyPlayer.generated.h"

class AGrappelBullet;
class ASwing;

UCLASS()
class SWINGERSALLOWED_API AMyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//public functions (other actors call these)
	void GrappelHit();
	void SwingHit();
	void SwingHasBase(float Height);
	
	//public variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int KeyCount = 0;
	UPROPERTY(EditAnywhere)
	FVector InitialSpawnLocation = FVector (0,0,110.15);
	FVector SwingPivot = FVector (230,-600,200);
	FVector InitialGrappelPosition = FVector (0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SwingRadius = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationRate = 100;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MovementType = 0;

private:
	//General Variables

	int t = 0;
	int T = 0;
	UPROPERTY(EditAnywhere)
	float MaxRange = 1000;

	UPROPERTY(EditAnywhere)
	TArray<ATriggerVolume*> RespawnLocations;

	//General Functions

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void LookUpRate(float AxisValue);
	void LookRightRate(float AxisValue);
	void CustomJump();
	void Release();
	void Move(int Type);
	void Die();
	void OpenChest();
	void SetRespawnPoint();
	
	//Grappel Variables & Functions

	UPROPERTY(EditAnywhere)
	float GrappelSpeed = 0.1;
	bool bGrappelHit = false;
	float GrappelScaleFactor = 0;
	
	FVector GrappelDestination = FVector (1, 1, 1);
	FVector GrappelPosition = FVector (0,0,0);
	FVector GrappelVelocity = FVector (0,0,0);
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGrappelBullet> GrappelBullet;
	AGrappelBullet* TempGrappel = nullptr;

	void ShootGrappelBullet();
	void GrappelMovement();
	void CalculateGrappelPosition(int a);
	
	
	//Swing Variables & Functions

	UPROPERTY(EditAnywhere)
	float AngularPosition = 0.01;
	float AngularSpeed = 0;
	float MaxAngle = 0;
	float XYangle = 0;
	int PreviousMovementType = 0;
	bool bSwingHit = false;
	bool bInitialGrappelComplete = false;
	bool bInitialHeight = false;
	bool bSwingBase = false;
	float PlayerHeight = 90.2;

	FVector InitialSwingPosition = FVector (0,0,0);
	FVector SwingVelocity = FVector (0,0,0);
	float HeightOfBase;
	
	ASwing* TempSwing = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ASwing> SwingBullet;
	
	void ShootSwingBullet();
	
	void SwingingMovement();
	void CalculateSwingingAngularPosition();
	FVector CalculateSwingingPosition();
};
