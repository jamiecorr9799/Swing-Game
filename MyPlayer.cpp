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
	
	Release(); //Stop any grappeling/swinging
}

void AMyPlayer::Release()
{
	if(!TempGrappel && !TempSwing)
	{
		return;
	}

	if(MovementType == 1) //if player has just finished grappling
	{
		GetCharacterMovement()->Velocity = GrappelVelocity; //set velocity to same as when grappling
	}

	if(MovementType == 2) //if player has just finished swinging 
	{
		GetCharacterMovement()->Velocity = SwingVelocity; //set velocity to same as when swinging
	}

	MovementType = 0; //set player movement to walking
	 
	if(IsValid(TempGrappel))
	{
		if(TempGrappel->IsValidLowLevel())
		{	
			TempGrappel->Release(); //resets movement components on blocks
			TempGrappel->Destroy(); //destroy grappel bullet
		}
	}
	
	if(IsValid(TempSwing))
	{
		if(TempSwing->IsValidLowLevel())
		{	
			TempSwing->Release(); //resets movement components on blocks
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
		if(bGrappelHit) //and if grappel bullet is attached
		{
			GrappelMovement(); //grappel
		}
	}

	if(Type == 2) //if swinging movement
	{
		if(bSwingHit) //and if swinging bullet is attached
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

		FVector SpawnLocation = GetActorLocation() + 100 * PlayerRotation.Vector(); //spawn location set to players location but out of reach

		TempGrappel = GetWorld()->SpawnActor<AGrappelBullet>(GrappelBullet, SpawnLocation, GetActorRotation()); //spawn grappel bullet at spawn location with the player's rotation
		TempGrappel->SetOwner(this); //the player owns the grapple bullet
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
	MovementType = 1; ///change movement to grappling
	
	InitialGrappelPosition = GetActorLocation(); //intial position is where the player is
	GrappelDestination = TempGrappel->GetActorLocation(); //destination is where the grapple is
	GrappelScaleFactor = (GrappelDestination - InitialGrappelPosition).Size(); //distance between player and grapple (used to adjust speed)

	if(GrappelScaleFactor > MaxRange) //if the grapple is too far away
	{
		MovementType = 0; //set movement to walking
		Release(); //release the grapple
	}

	GrappelVelocity = 50 * (GrappelDestination - InitialGrappelPosition) * (GrappelSpeed) * ( 1 / GrappelScaleFactor ); //calculates player velocity when grappeling
	
}

void AMyPlayer::GrappelMovement()
{	
	if(!TempGrappel)
	{
		return;
	}

	if(GrappelSpeed * (t / GrappelScaleFactor) < 0.95) //if player isn't at grapple destination
	{
		CalculateGrappelPosition(t);

		t = t + 1; 
	}

	if(GrappelSpeed * (t / GrappelScaleFactor) > 0.95) //if player is at grapple location, slow down grapple to a stop
	{
		GrappelVelocity = FVector (0,0,0);
	}

	SetActorLocation(GrappelPosition);
}

void AMyPlayer::CalculateGrappelPosition(int a)
{
	GrappelPosition = (1 - GrappelSpeed * (a / GrappelScaleFactor)) * InitialGrappelPosition + GrappelSpeed * (a/ GrappelScaleFactor) * GrappelDestination; //move towards grapple destination
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

		FVector SpawnLocation = GetActorLocation() + 100 * PlayerRotation.Vector();	//spawn location set to player's location and out of reach

		TempSwing = GetWorld()->SpawnActor<ASwing>(SwingBullet, SpawnLocation, GetActorRotation());	//spawn swing bullet at spawn location with the player's rotation
		TempSwing->SetOwner(this);	//the player owns the swing bullet
	}
	
	T = 0;
	t = 0;
	bSwingHit = false;	//Swing hasn't attached yet
	bInitialGrappelComplete = false;
	//bInitialHeight = false;
	bSwingBase = false;
}

void AMyPlayer::SwingHit()
{	
	if(!TempSwing)
	{
		return;
	}

	bSwingHit = true;	//swing has attached	

	InitialSwingPosition = GetActorLocation();	//gets location of where swing starts (player's location)
	SwingPivot = TempSwing->GetActorLocation();	//gets location of pivot

	float Radius = (InitialSwingPosition - SwingPivot).Size();

	if(Radius > MaxRange) //if swing distance is too far
	{
		MovementType = 0; //set movement to walking
		Release(); //release the swing
	}

	//calulates the angle between the player and pivot (if pivot had same Z value as player) on the XY plane
	FVector A = FVector( InitialSwingPosition.X - SwingPivot.X, InitialSwingPosition.Y - SwingPivot.Y, 0);	
	FVector B =  FVector (1, 0, 0);
	XYangle = FMath::Acos( A.X / (A.Size() * B.Size()));


	if(InitialSwingPosition.Y - SwingPivot.Y < 0) //if the swing pivot is above the player
	{
		XYangle = 2*PI - XYangle; 	//adjusts the angle to the true value since ACos isn't a 1 to 1 function

	}	
	

	//calculates the angle between the player and pivot on the Z plane  (i.e. the boundary angles the player will be swinging between) 
	FVector C = InitialSwingPosition - SwingPivot;
	FVector D = FVector (0, 0,InitialSwingPosition.Z - SwingPivot.Z);
	MaxAngle =  FMath::Acos( C.Z * D.Z / ( C.Size() * D.Size() ) );

	//calculates radius of swing
	SwingRadius = (InitialSwingPosition - SwingPivot).Size();
	
	AngularPosition = 0;

	if(!bInitialGrappelComplete) //if player hasn't yet completed the grapple (i.e. player isn't ready to swing and needs to grapple towards the pivot to ensure they dont immediately hit the floor)
	{
		//bInitialHeight = true;

		MovementType = 2; //set movement to swinging

		InitialGrappelPosition = GetActorLocation();
		
		if(IsValid(TempSwing))
		{	
			GrappelDestination = TempSwing->GetActorLocation(); // grappling destination is the swing pivot
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
			CalculateGrappelPosition(T); //calculates movement of player whilst grappling

			T++; 

			SetActorLocation(GrappelPosition);	

			SwingRadius = (SwingPivot - GetActorLocation()).Size();
		}

		if(SwingPivot.Z - SwingRadius - PlayerHeight > HeightOfBase && bInitialGrappelComplete == false) //if player has finished grappeling
		{
			bInitialGrappelComplete = true; //the player can now start swinging
			SwingHit(); //updates key variables to calculate swinging movement
		}
				
		if(SwingPivot.Z - SwingRadius - PlayerHeight > HeightOfBase && bInitialGrappelComplete == true) //if player has finished grappelling and is ready to swing
		{
			CalculateSwingingAngularPosition(); //calculates speed of player whilst swinging
			
			FVector SwingPosition = CalculateSwingingPosition(); //caluclates movement of player whilst swinging

			SetActorLocation(SwingPosition);
			
			t++;
		}
	}


	else if(InitialGrappelPosition.Z - SwingPivot.Z < 0 && bSwingBase == false) //swing has no base and pivot is above player
	{
		if(GrappelSpeed * (T / GrappelScaleFactor) < 0.4) //if player is doing the initial grappel
		{
			CalculateGrappelPosition(T);  //calculates movement of player whilst grappling

			T++; 

			SetActorLocation(GrappelPosition);	
		}

		if(GrappelSpeed * (T / GrappelScaleFactor) > 0.4 && bInitialGrappelComplete == false) //if player has finished initial grappel
		{
			bInitialGrappelComplete = true;	//the player can now start swinging
			SwingHit(); //updates key variables to calculate swinging movement
		}
				
		if(GrappelSpeed * (T / GrappelScaleFactor) > 0.4 && bInitialGrappelComplete == true) //if player is ready to swing
		{
			CalculateSwingingAngularPosition(); //calculates speed of player whilst swinging
			
			FVector SwingPosition = CalculateSwingingPosition(); //caluclates movement of player whilst swinging

			SetActorLocation(SwingPosition);
			
			t++;

		}
	}
}

void AMyPlayer::CalculateSwingingAngularPosition() //calculates the angle between the player and pivot in the Z-plane     (i.e. how far through the swing the player is)
{
	if(InitialSwingPosition.Z - SwingPivot.Z < 0) //if swing pivot is above the player
		{
			AngularPosition = -MaxAngle * FMath::Cos(t * 0.1 * sqrt(10 / SwingRadius)); //rotation rate changes depending where it is on the cycle (enables pivot to go back and forth)
			AngularSpeed = MaxAngle * sqrt(10 / SwingRadius) * FMath::Sin(t * 0.1 * sqrt(10 / SwingRadius));
		}

	else  //if swing pivot height is below the player
		{
			AngularPosition = - (PI - MaxAngle) * FMath::Cos(t * 0.1 * sqrt(10 / SwingRadius)) + (PI - MaxAngle); //rotation rate changes depending where it is on the cycle (enables pivot to go back and forth)
			AngularSpeed = (PI - MaxAngle) * sqrt(10 / SwingRadius) * FMath::Sin(t * 0.1 * sqrt(10 / SwingRadius));
		}
}

FVector AMyPlayer::CalculateSwingingPosition()
{	
	FVector SwingPosition = FVector (0,0,0);
	
	if(InitialSwingPosition.Z - SwingPivot.Z < 0) //if swing pivot is above the player
		{
			SwingPosition = FVector (SwingRadius * FMath::Sin(-AngularPosition) * FMath::Cos(XYangle) + SwingPivot.X , SwingRadius * FMath::Sin(-AngularPosition) * FMath::Sin(XYangle) + SwingPivot.Y, - SwingRadius * FMath::Cos(-AngularPosition) + SwingPivot.Z);
			SwingVelocity = - 10 * FVector (SwingRadius * AngularSpeed * FMath::Cos(-AngularPosition) * FMath::Cos(XYangle), SwingRadius * AngularSpeed * FMath::Cos(-AngularPosition) * FMath::Sin(XYangle), SwingRadius * AngularSpeed * FMath::Sin(-AngularPosition));
		}
	else //if swing pivot height is below the player
		{
			SwingPosition = FVector (SwingRadius * FMath::Sin(AngularPosition + MaxAngle) * FMath::Cos(XYangle) + SwingPivot.X , SwingRadius * FMath::Sin(AngularPosition + MaxAngle) * FMath::Sin(XYangle) + SwingPivot.Y, SwingRadius * FMath::Cos(AngularPosition + MaxAngle) + SwingPivot.Z);
			SwingVelocity = 10 * FVector (SwingRadius * AngularSpeed * FMath::Cos(AngularPosition + MaxAngle) * FMath::Cos(XYangle), SwingRadius * AngularSpeed * FMath::Cos(AngularPosition + MaxAngle) * FMath::Sin(XYangle), - SwingRadius * AngularSpeed * FMath::Sin(AngularPosition + MaxAngle));
		}

	return SwingPosition;
}

void AMyPlayer::Die()
{
	if(GetActorLocation().Z < -5000) //if the player has fallen off the plaform 
	{
		SetActorLocation(InitialSpawnLocation); //respawn player at last visited spawn point
	}
}

void AMyPlayer::SetRespawnPoint()
{
	if(RespawnLocations.Num() < 1) //if there are no respawn points
	{
		return;
	}

	for(ATriggerVolume* SpawnPoint: RespawnLocations) //for each respawn location 
	{	
		TArray<AActor*> ActorsOnRespawnPoint;
		SpawnPoint->GetOverlappingActors(ActorsOnRespawnPoint); //identify all actors in respawn location

		for(AActor* Body: ActorsOnRespawnPoint) 
		{
			AMyPlayer* PlayerPawn = Cast<AMyPlayer>(Body);

			if(PlayerPawn) //if one of the actors (in the respawn location) is the player 
			{
				InitialSpawnLocation = GetActorLocation(); //set respawn point
			}
		}
	}
}

void AMyPlayer::SwingHasBase(float Height)
{
	bSwingBase = true;
	HeightOfBase = Height;
}

