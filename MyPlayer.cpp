// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayer.h"
#include "Math/UnrealMathUtility.h"
#include "SwingersAllowed/Grappel/GrappelBullet.h"
#include "SwingersAllowed/Swing/Swing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"




// Sets default values
AMyPlayer::AMyPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	Move(MovementType);
	Die();
	
}

// Called to bind functionality to input
void AMyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this,  &AMyPlayer::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMyPlayer::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMyPlayer::LookUpRate);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &AMyPlayer::LookRightRate);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AMyPlayer::CustomJump);
	PlayerInputComponent->BindAction(TEXT("Swing"), EInputEvent::IE_Pressed, this, &AMyPlayer::ShootSwingBullet);
	PlayerInputComponent->BindAction(TEXT("Grappel"), EInputEvent::IE_Pressed, this, &AMyPlayer::ShootGrappelBullet);
	PlayerInputComponent->BindAction(TEXT("Release"), EInputEvent::IE_Released, this, &AMyPlayer::Release);
	

}

void AMyPlayer::MoveForward(float AxisValue)
{ 
	if(MovementType == 0 || MovementType == 4)
	{
		AddMovementInput(GetActorForwardVector() * AxisValue);
	}
}

void AMyPlayer::MoveRight(float AxisValue)
{	
	if(MovementType == 0 || MovementType == 4)
	{
		AddMovementInput(GetActorRightVector() * AxisValue);
	}
}

void AMyPlayer::LookUpRate(float AxisValue)
{
	AddControllerPitchInput(AxisValue * RotationRate * GetWorld() -> GetDeltaSeconds());
}

void AMyPlayer::LookRightRate(float AxisValue)
{
	AddControllerYawInput(AxisValue * RotationRate * GetWorld()->GetDeltaSeconds());
}

void AMyPlayer::CustomJump()
{		
	Jump();
	
	//Stop any grappeling/swinging
	Release();
}

void AMyPlayer::Release()
{
	if(!TempGrappel && !TempSwing)
	{
		return;
	}

	if(MovementType == 1)
	{
		GetCharacterMovement()->Velocity = GrappelVelocity;
	}

	if(MovementType == 2)
	{
		GetCharacterMovement()->Velocity = SwingVelocity;
	}

	MovementType = 0;
	 
	if(IsValid(TempGrappel))
	{
		if(TempGrappel->IsValidLowLevel())
		{	
			TempGrappel->Release();
			TempGrappel->Destroy(); //destroy grappel bullet
		}
	}
	
	if(IsValid(TempSwing))
	{
		if(TempSwing->IsValidLowLevel())
		{	
			TempSwing->Release();
			TempSwing->Destroy(); //destroy swinging bullet
		}
	}
	
	t = 0;
	T = 0;

	TempSwing = nullptr;
	TempGrappel = nullptr;
}

void AMyPlayer::Move(int Type)
{
	if(Type == 1) //if grappel movement
	{
		if(bGrappelHit == true) //and if grappel bullet has attached
		{
			GrappelMovement(); //grappel
		}
	}

	if(Type == 2) //if swinging movement
	{
		if(bSwingHit) //and if swinging bullet has attached
		{
			SwingingMovement(); //swing
		}
	}
}

void AMyPlayer::ShootGrappelBullet()
{	
	if(MovementType == 1 || MovementType == 2)
	{
		return;
	}

	if(GrappelBullet)
	{
		FVector PlayerLocation;
		FRotator PlayerRotation;
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(PlayerLocation, PlayerRotation); //get location and rotation of where player is looking

		FVector SpawnLocation = GetActorLocation() + 100 * PlayerRotation.Vector(); //spawn location set to current location and out of reach

		TempGrappel = GetWorld()->SpawnActor<AGrappelBullet>(GrappelBullet, SpawnLocation, GetActorRotation()); //spawn grappel bullet at spawn location with the player's rotation
		TempGrappel->SetOwner(this); //the player owns the grappel bullet
	}

	t = 0;
	bGrappelHit = false; //grappel hasn't attached yet
}

void AMyPlayer::GrappelHit()
{	
	if(!TempGrappel)
	{
		return;
	}

	bGrappelHit	= true;	//grappel has attached
	MovementType = 1;
	
	InitialGrappelPosition = GetActorLocation(); //intial position is where the player is
	GrappelDestination = TempGrappel->GetActorLocation(); //destination is where the grappel is
	GrappelScaleFactor = (GrappelDestination - InitialGrappelPosition).Size(); //adjusts so speed is constant

	if(GrappelScaleFactor > MaxRange)
	{
		MovementType = 0;
		Release();
	}

	GrappelVelocity = 50 * (GrappelDestination - InitialGrappelPosition) * (GrappelSpeed) * ( 1 / GrappelScaleFactor ); //calculates player velocity when grappeling
	
}

void AMyPlayer::GrappelMovement()
{	
	if(!TempGrappel)
	{
		return;
	}

	if(GrappelSpeed * (t / GrappelScaleFactor) < 0.95) //if player isn't at grappel destination
	{
		CalculateGrappelPosition(t);

		t = t + 1; 
	}

	if(GrappelSpeed * (t / GrappelScaleFactor) > 0.95)
	{
		GrappelVelocity = FVector (0,0,0);
	}

	SetActorLocation(GrappelPosition);
}

void AMyPlayer::CalculateGrappelPosition(int a)
{
	GrappelPosition = (1 - GrappelSpeed * (a / GrappelScaleFactor)) * InitialGrappelPosition + GrappelSpeed * (a/ GrappelScaleFactor) * GrappelDestination; //move towards grappel destination
}

void AMyPlayer::ShootSwingBullet()
{	
	if(MovementType == 1 || MovementType == 2)
	{
		return;
	}

	if(SwingBullet)
	{
		FVector PlayerLocation;
		FRotator PlayerRotation;
		GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(PlayerLocation, PlayerRotation); //get location and rotation of where player is looking

		FVector SpawnLocation = GetActorLocation() + 100 * PlayerRotation.Vector();	//spawn location set to current location and out of reach

		TempSwing = GetWorld()->SpawnActor<ASwing>(SwingBullet, SpawnLocation, GetActorRotation());	//spawn swing bullet at spawn location with the player's rotation
		TempSwing->SetOwner(this);	//the player owns the swing bullet
	}
	
	T = 0;
	t = 0;
	bSwingHit = false;	//Swing hasn't attached yet
	bInitialGrappelComplete = false;
	bInitialHeight = false;
	bSwingBase = false;
}

void AMyPlayer::SwingHit()
{	
	if(!TempSwing)
	{
		return;
	}

	bSwingHit = true;	//swing has attached	

	InitialSwingPosition = GetActorLocation();	//gets location of where swing starts
	SwingPivot = TempSwing->GetActorLocation();	//gets location of pivot

	float Radius = (InitialSwingPosition - SwingPivot).Size();

	if(Radius > MaxRange)
	{
		MovementType = 0;
		Release();
	}

	//calulates the angle between the player and pivot (if pivot had same Z value as player) on the XY plane
	FVector A = FVector( InitialSwingPosition.X - SwingPivot.X, InitialSwingPosition.Y - SwingPivot.Y, 0);	
	FVector B =  FVector (1, 0, 0);
	XYangle = FMath::Acos( A.X / (A.Size() * B.Size()));


	//adjusts the angle to the true value since ACos isn't a 1 to 1 function
	if(InitialSwingPosition.Y - SwingPivot.Y < 0)
	{
		XYangle = 2*PI - XYangle;
	}	
	

	//calculates the angle between the player and pivot on the Z plane  (i.e. the boundary angles the player will be swinging between) 
	FVector C = InitialSwingPosition - SwingPivot;
	FVector D = FVector (0, 0,InitialSwingPosition.Z - SwingPivot.Z);
	MaxAngle =  FMath::Acos( C.Z * D.Z / ( C.Size() * D.Size() ) );

	//calculates radius of swing
	SwingRadius = (InitialSwingPosition - SwingPivot).Size();
	
	AngularSpeed = 0;

	if(bInitialGrappelComplete == false)
	{
		bInitialHeight = true;

		PreviousMovementType = MovementType;
		MovementType = 2;

		InitialGrappelPosition = GetActorLocation(); //intial position is where the player is
		if(IsValid(TempSwing))
		{	
			GrappelDestination = TempSwing->GetActorLocation(); //destination is where the grappel is
		}
		GrappelScaleFactor = (GrappelDestination - InitialGrappelPosition).Size(); //adjusts so speed is constant
		SwingVelocity = 50 * (GrappelDestination - InitialGrappelPosition) * (GrappelSpeed) * ( 1 / GrappelScaleFactor );	
	}
}

void AMyPlayer::SwingingMovement()
{	
	if(!TempSwing) //if swing exists
	{
		return;
	}

	if(bSwingBase == true && InitialGrappelPosition.Z - SwingPivot.Z < 0) //if grappel has a base AND the pivot is above the player 
	{
		if(SwingPivot.Z - SwingRadius - PlayerHeight < HeightOfBase) //if player needs to grappel before swinging
		{
			CalculateGrappelPosition(T);

			T++; 

			SetActorLocation(GrappelPosition);	

			SwingRadius = (SwingPivot - GetActorLocation()).Size();
		}

		if(SwingPivot.Z - SwingRadius - PlayerHeight > HeightOfBase && bInitialGrappelComplete == false) //if player has finished grappeling
		{
			bInitialGrappelComplete = true;
			SwingHit();
		}
				
		if(SwingPivot.Z - SwingRadius - PlayerHeight > HeightOfBase && bInitialGrappelComplete == true) //if player has finished grappelling and is ready to swing
		{
			//once initial grappeling is complete
			FVector SwingPosition = FVector (0,0,0);

			CalculateSwingingAngularSpeed();
			
			SwingPosition = CalculateSwingingPosition();

			SetActorLocation(SwingPosition);
			
			t++;

		}
	}


	else if(InitialGrappelPosition.Z - SwingPivot.Z < 0 && bSwingBase == false) //swing has no base and pivot is above player
	{
		if(GrappelSpeed * (T / GrappelScaleFactor) < 0.4) //if player is doing the initial grappel
		{
			CalculateGrappelPosition(T);

			T++; 

			SetActorLocation(GrappelPosition);	
		}

		if(GrappelSpeed * (T / GrappelScaleFactor) > 0.4 && bInitialGrappelComplete == false) //if player has finished initial grappel
		{
			bInitialGrappelComplete = true;
			SwingHit();
		}
				
		if(GrappelSpeed * (T / GrappelScaleFactor) > 0.4 && bInitialGrappelComplete == true) //if player is ready to swing
		{
			//once initial grappeling is complete
			FVector SwingPosition = FVector (0,0,0);

			CalculateSwingingAngularSpeed();
			
			SwingPosition = CalculateSwingingPosition();

			SetActorLocation(SwingPosition);
			
			t++;

		}
	}
}

void AMyPlayer::CalculateSwingingAngularSpeed()
{
	if(InitialSwingPosition.Z - SwingPivot.Z < 0) //if swing pivot is above the player
		{
			AngularSpeed = -MaxAngle * FMath::Cos(t * 0.1 * sqrt(10 / SwingRadius)); //rotation rate changes depending where it is on the cycle (enables pivot to go back and forth)
			AngularAcceleration = MaxAngle * sqrt(10 / SwingRadius) * FMath::Sin(t * 0.1 * sqrt(10 / SwingRadius));
		}

	else  //if swing pivot height is below the player
		{
			AngularSpeed = - (PI - MaxAngle) * FMath::Cos(t * 0.1 * sqrt(10 / SwingRadius)) + (PI - MaxAngle); //rotation rate changes depending where it is on the cycle (enables pivot to go back and forth)
			AngularAcceleration = (PI - MaxAngle) * sqrt(10 / SwingRadius) * FMath::Sin(t * 0.1 * sqrt(10 / SwingRadius));
		}
}

FVector AMyPlayer::CalculateSwingingPosition()
{	
	FVector SwingPosition = FVector (0,0,0);
	
	if(InitialSwingPosition.Z - SwingPivot.Z < 0) //if swing pivot is above the player
		{
			SwingPosition = FVector (SwingRadius * FMath::Sin(-AngularSpeed) * FMath::Cos(XYangle) + SwingPivot.X , SwingRadius * FMath::Sin(-AngularSpeed) * FMath::Sin(XYangle) + SwingPivot.Y, - SwingRadius * FMath::Cos(-AngularSpeed) + SwingPivot.Z);
			SwingVelocity = - 10 * FVector (SwingRadius * AngularAcceleration * FMath::Cos(-AngularSpeed) * FMath::Cos(XYangle), SwingRadius * AngularAcceleration * FMath::Cos(-AngularSpeed) * FMath::Sin(XYangle), SwingRadius * AngularAcceleration * FMath::Sin(-AngularSpeed));
		}
	else //if swing pivot height is below the player
		{
			SwingPosition = FVector (SwingRadius * FMath::Sin(AngularSpeed + MaxAngle) * FMath::Cos(XYangle) + SwingPivot.X , SwingRadius * FMath::Sin(AngularSpeed + MaxAngle) * FMath::Sin(XYangle) + SwingPivot.Y, SwingRadius * FMath::Cos(AngularSpeed + MaxAngle) + SwingPivot.Z);
			SwingVelocity = 10 * FVector (SwingRadius * AngularAcceleration * FMath::Cos(AngularSpeed + MaxAngle) * FMath::Cos(XYangle), SwingRadius * AngularAcceleration * FMath::Cos(AngularSpeed + MaxAngle) * FMath::Sin(XYangle), - SwingRadius * AngularAcceleration * FMath::Sin(AngularSpeed + MaxAngle));
		}

	return SwingPosition;
}

void AMyPlayer::Die()
{
	if(GetActorLocation().Z < -5000)
	{
		SetActorLocation(InitialSpawnLocation);
	}
}

void AMyPlayer::SetRespawnPoint()
{
	if(RespawnLocations.Num() < 1)
	{
		return;
	}

	for(ATriggerVolume* SpawnPoint: RespawnLocations)
	{	
		TArray<AActor*> ActorsOnRespawnPoint;
		SpawnPoint->GetOverlappingActors(ActorsOnRespawnPoint);

		for(AActor* Body: ActorsOnRespawnPoint)
		{
			AMyPlayer* PlayerPawn = Cast<AMyPlayer>(Body);

			if(PlayerPawn)
			{
				InitialSpawnLocation = GetActorLocation();
			}
		}
	}
}

void AMyPlayer::SwingHasBase(float Height)
{
	bSwingBase = true;
	HeightOfBase = Height;
}

