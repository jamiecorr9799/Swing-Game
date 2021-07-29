// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwingersAllowed/BlockComponents/StandardBlock.h"
#include "SwingersAllowed/Players/MyPlayer.h"
#include "Swing.generated.h"

UCLASS()
class SWINGERSALLOWED_API ASwing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BulletMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Release();

private:

	bool bHit = false;

	UPROPERTY(EditAnywhere)
	float BulletSpeed = 10;
	
	FRotator BulletDirection = FRotator (0, 0, 0);
	FVector PlayerPosition = FVector (0, 0, 0);

	AStandardBlock* Block = nullptr;

	AMyPlayer* PlayerPawn = nullptr;

	AActor* MoveForwardActor = nullptr;

	void Move();
	

};
