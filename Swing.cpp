// Fill out your copyright notice in the Description page of Project Settings.


#include "Swing.h"
#include "Components/StaticMeshComponent.h"
#include "SwingersAllowed/Swing/SwingingPlatform.h"
#include "SwingersAllowed/BlockComponents/StandardBlock.h"
#include "SwingersAllowed/BlockComponents/MoveForwardComponent.h"
#include "Engine/TriggerVolume.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASwing::ASwing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh")); 
	RootComponent = BulletMesh;
}

// Called when the game starts or when spawned
void ASwing::BeginPlay()
{
	Super::BeginPlay();
	
	BulletMesh->OnComponentBeginOverlap.AddDynamic(this, &ASwing::OnOverlapBegin);

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(PlayerPosition, BulletDirection);

	PlayerPawn = Cast<AMyPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
}

// Called every frame
void ASwing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Move();
}

void ASwing::Move()
{
	if(bHit == false)
	{	
		FVector Position = GetActorLocation();
		Position = Position + BulletSpeed * BulletDirection.Vector();
		SetActorLocation(Position);
	}
}

void ASwing::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	ATriggerVolume* PressurePlate = Cast<ATriggerVolume>(OtherActor); //if swing hits trigger volume, return

	if(PressurePlate)
	{
		return;
	}

	if(bHit == false) //on swings first hit
	{
		bHit = true;
		
		AMyPlayer* Player = Cast<AMyPlayer>(GetOwner()); //if swing hits the player, return
		
		if(!Player)
		{
			return;
		}
			
		ASwingingPlatform* SwingWithBase = Cast<ASwingingPlatform>(OtherActor); //if swing hits platform with base, tell the player swing height of base and return
		if(SwingWithBase) 
		{	
			Player->SwingHasBase(SwingWithBase->GetActorLocation().Z + 20);
			Player->SwingHit();
			return;
		}
		
		Player->SwingHit(); //if it hasn't hit any special actors, tell player its hit
	

		Block = Cast<AStandardBlock>(OtherActor); //if swing has hit a standard block, disable the movement component

		if(IsValid(Block))
		{
			if(Block->bMovementComponent)
			{
				Block->bMovementEnabled = false;
			}
		}

		// else
		// {
		// 	Block = nullptr;
		// }
	
		UActorComponent* ForwardComponent = OtherActor->FindComponentByClass(UMoveForwardComponent::StaticClass()); //if platform has movement component, make platform stationary and save reference

		if(IsValid(ForwardComponent))
		{
			UMoveForwardComponent* MoveForwardComponent = Cast<UMoveForwardComponent>(ForwardComponent);

			if(IsValid(MoveForwardComponent))
			{
				MoveForwardComponent->bStationary = true;  
				MoveForwardActor = OtherActor;
				return;
			}
		}
		
	}
}

void ASwing::Release()
{	

	if(IsValid(Block)) //if releasing from block, set movement on
	{	
		if(Block->IsValidLowLevel())
		{	
			if(Block->bMovementComponent) 
			{
				Block->bMovementEnabled = true; 
			}
		}
	}

	if(IsValid(MoveForwardActor))
	{	
		if(MoveForwardActor->IsValidLowLevel())
		{	
			UActorComponent* ForwardComponent = MoveForwardActor->FindComponentByClass(UMoveForwardComponent::StaticClass());     // BUGGG

			if(IsValid(ForwardComponent))
			{
				UMoveForwardComponent* MoveForwardComponent = Cast<UMoveForwardComponent>(ForwardComponent);
				
				if(IsValid(MoveForwardComponent))
				{
					MoveForwardComponent->bStationary = false;
				}
			}
		}
		
	}

	Block = nullptr;
	MoveForwardActor = nullptr;
}

