// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "DynamicPawn.h"
#include "MyPawn.h"
#include "Bullet.h"

// Sets default values
AMyPawn::AMyPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create a dummy root component we can attach things to.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	// Create a visible object
	OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
	// Attach visible object to our root component.
	OurVisibleComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	FVector origin, bounds;
	//initialising int and float variables for the start of the game
	level = 1;
	ballNum = 3;
	multiplier = 1.0f;	
	ballSpeed = 5.0f;
	score = 0;	
	numBallsRemaining = 0;
	platformCount = NUMBOXWIDTH * NUMBOXHEIGHT; //number of boxes in a row * number of boxes in column


	//initialising bools for the start of the game
	bMainBall = false;
	bMultiplierCanEarnBall = true;
	timerCount = 0.0f;
	bTimerActive = false;

	//initialising strings for UI
	bonusBallString = "";
	tutorialText = "Press 'E' or 'Space' to spawn a ball. \n'A' and 'D' or arrow keys to move";
	gameOverText = "";

	Super::BeginPlay();

	// Always good practice to initialise your variables to ensure nothing odd happens
	currentDeltaTime = 0.0f;

	//code for static camera
	OurCamera = (ACameraActor*)(GetWorld()->SpawnActor(ACameraActor::StaticClass())); //create a camera actor
	CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); //find the camaera manager linked to the player controller
	if (OurCamera && CameraManager) //if both have been sucessful
	{
		OurCamera->SetActorLocation(FVector(-4250.0f, 0.0f, 2000.0f)); //set location of camera actor
		OurCamera->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f)); //set rotation of camera actor
		CameraManager->SetViewTarget(OurCamera); //set viewport target of camera manager
	}

	OurVisibleComponent->SetWorldLocation(FVector(0.0f, 0.0, 50.0f)); //set location of player paddle
	OurVisibleComponent->SetWorldScale3D(FVector(2.5f, 8.0f, 1.0f)); //set scale of player paddle

	//spawn floor and roof platform
	APlatformBox *SpawnedPlatform = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass());
	SpawnedPlatform->SetScale(FVector(2.5f, 40.0f, 0.5f));
	SpawnedPlatform->SetPosition(FVector(0.0f, 0.0f, 0.0f));

	SpawnedPlatform = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass());
	SpawnedPlatform->SetScale(FVector(2.5f, 40.0f, 0.5f));
	SpawnedPlatform->SetPosition(FVector(0.0f, 0.0f, 3925.0f));
	
	//spawn wall platforms
	SpawnedPlatform = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass());
	SpawnedPlatform->SetRotation(FRotator(0.0f, 0.0f, 90.0f));
	SpawnedPlatform->SetScale(FVector(2.5f, 39.0f, 0.5f));
	SpawnedPlatform->SetPosition(FVector(0.0f, LEVELWIDTH, 1960.0f));
	
	SpawnedPlatform = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass());
	SpawnedPlatform->SetRotation(FRotator(0.0f, 0.0f, -90.0f));
	SpawnedPlatform->SetScale(FVector(2.5f, 39.0f, 0.5f));
	SpawnedPlatform->SetPosition(FVector(0.0f, -LEVELWIDTH, 1960.0f));
	
	//spawn destructible platforms based on level value
	SpawnPlatforms(level); 
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	currentDeltaTime = DeltaTime;

	FVector NewLocation;

	// Handle movement based on our "MoveY" axis
	{
		if (!MoveVelocity.IsZero())
		{
			NewLocation = GetActorLocation() + (MoveVelocity * DeltaTime * MOVESPEED * 1000.0f);

			//limit movement to within the level width
			if (NewLocation.Y >= (LEVELWIDTH - ACTORWIDTH) || NewLocation.Y <= (-LEVELWIDTH + ACTORWIDTH))
			{
				NewLocation.Y = GetActorLocation().Y;
				MoveVelocity.Y = MoveVelocity.Y * -1.0f * 0.25f; //perform a small bounce in the opposite direction
			}

			SetActorLocation(NewLocation);
		}
	}

	//turn off bonus ball text displayed on screen
	if (bTimerActive && timerCount > 0.0f)
	{
		timerCount -= 1 * currentDeltaTime;
		if (timerCount <= 0.0f) //if timer has finished then reset values of timer and text string
		{
			bTimerActive = false;
			timerCount = 0.0f;
			bonusBallString = "";
		}
	}

}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Respond every frame to the values of our movement axis, "MoveY".
	InputComponent->BindAxis("MoveY", this, &AMyPawn::Move_YAxis);

	// Respond to a fire button
	InputComponent->BindAction("FireFast", IE_Pressed, this, &AMyPawn::SpawnMainBall);
}

void AMyPawn::Move_YAxis(float AxisValue)
{
	// Accelerate at 100 units per second per second right or left
	CurrentVelocity.Y += FMath::Clamp(AxisValue, -1.0f, 1.0f) * 300.0f * currentDeltaTime;

	// Clamp to a maximum velocity of 500 units per second in this axis
	CurrentVelocity.Y = FMath::Clamp(CurrentVelocity.Y, -500.0f, 500.0f);
	if (AxisValue < 0)
	{
		MoveVelocity.Y = -1.0f;
	}
	if (AxisValue > 0)
	{
		MoveVelocity.Y = 1.0f;
	}
	// if no input then reduce current velocity to 0
	if (AxisValue == 0)
	{
		MoveVelocity.Y = 0.0f;
		if (CurrentVelocity.Y != 0.0f)
		{
			CurrentVelocity.Y *= 0.75f;
		}

	}
}

//function to spawn a main ball when 'E', 'Space' or 'up arrow' are pressed
void AMyPawn::SpawnMainBall() 
{
	if (ballNum > 0 && !bMainBall) //if no main ball exists in the scene and there is a ball to spawn
	{
		tutorialText = ""; //sets tutorial text to blank to 'toggle off' UI
		gameOverText = ""; //sets game over text to blank to 'toggle off' UI
		float ballDirection; //creates a float value that is used to give the ball a Y direction to its velocity
		if (CurrentVelocity.Y <= 25.0f && CurrentVelocity.Y >= -25.0f) //deadzone for a straight shot if paddle is still slightly moving
		{
			ballDirection = 0.0f; //vertical shot
		}
		else
		{
			ballDirection = FMath::Clamp(CurrentVelocity.Y, -0.95f, 0.95f); //clamped to stop a completely horizontal shot
		}
		FVector fireDirection = FVector(0.0f, ballDirection, 1.0f); //set a direction to fire ball (if player hasn't moved, default is vertical)

		// Spawn a new ball
		ABullet* SpawnedBall = (ABullet*)GWorld->SpawnActor(ABullet::StaticClass());
		SpawnedBall->SetActorLocation(this->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f)); //set location of ball to be 150 above player paddle (not noticable but stops uneccessary collisions)

		bMainBall = true; // a main ball now exists in scene
		SpawnedBall->ballType = 1; //give the ball a type 1 to indicate it is a main ball
		SpawnedBall->SetMyMaterial(mainBallMaterial); //set its material to unique material so player can spot it

		SpawnedBall->CalculateVelocity(ballSpeed*100.0f, fireDirection); //give the spawned ball a velocity and direction
		ballNum--; //decrease the number of balls available (lives)
	}
}

//function to spawn a regular bonus ball
void AMyPawn::SpawnBall()
{
		float ballDirection;
		if (CurrentVelocity.Y <= 25.0f && CurrentVelocity.Y >= -25.0f)
		{
			ballDirection = 0.0f;
		}
		else
		{
			ballDirection = FMath::Clamp(CurrentVelocity.Y, -0.95f, 0.95f);
		}
		FVector fireDirection = FVector(0.0f, ballDirection, 1.0f);

		// Spawn a new ball
		ABullet* SpawnedBall = (ABullet*)GWorld->SpawnActor(ABullet::StaticClass());
		SpawnedBall->SetActorLocation(this->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f));
		SpawnedBall->ballType = 2; //ball type two is used for bonus balls
		SpawnedBall->CalculateVelocity(ballSpeed*100.0f, fireDirection);
}

//function to spawn multi-balls on the mega bonus
void AMyPawn::SpawnFanBalls() 
{
	// Spawn 5 new balls
	for (int i = 0; i < 5; i++)
	{
		float ballDirection;

		switch (i) //each ball is given a different direction to create a fan effect
		{
		case 0: ballDirection = -0.75f; break;
		case 1: ballDirection = -0.25f; break;
		case 2: ballDirection = -0.0f; break;
		case 3: ballDirection = 0.25f; break;
		case 4: ballDirection = 0.75f; break;
		default: break;
		}

		FVector fireDirection = FVector(0.0f, ballDirection, 1.0f);
		ABullet* SpawnedBall = (ABullet*)GWorld->SpawnActor(ABullet::StaticClass());
		SpawnedBall->SetActorLocation(this->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f));
		SpawnedBall->ballType = 2; //ball type two is used for bonus balls
		SpawnedBall->CalculateVelocity(ballSpeed*100.0f, fireDirection);
	}
}

//function called to increase multiplier variable
void AMyPawn::IncrementMultiplier() 
{ 
	IncrementScore(); //calculates the score based off the current multiplier before multiplier is increased
	multiplier += 1.0f; //increment multiplier
	if (multiplier == 5.0f && bMultiplierCanEarnBall) // check for bonus ball combo
	{
		bMultiplierCanEarnBall = false; //bool to stop players earning more than one ball per combo
		bonusBallString = "Bonus Ball!"; //makes bonus ball text appear
		timerCount = 1.0f; //sets the time for the text to display for
		bTimerActive = true; //initiates the countdown timer for UI
		if (ballNum < 3) //if the player hasn't got full lives
		{
			ballNum++; //add an extra life
		}
		SpawnBall(); //spawn a bonus ball
	}
	if (multiplier == 10) //check for mega bonus combo
	{
		bonusBallString = "MEGA BONUS!"; //sets text string to mega bonus
		timerCount = 1.0f; //sets the time for the text to display for
		bTimerActive = true; //initiates the countdown timer
		ballNum = 3; //fills the players lives
		SpawnFanBalls(); //spawns the fan effect of balls to signify mega bonus
	}
}

//function called when a type 1 platform (destructible) is hit
void AMyPawn::DecreasePlatformCount() 
{
	platformCount--; //decrement variable for number of platforms active in scene
	if (platformCount <= 0) //confirmation check performed to ensure all platforms are destroyed
	{
		for (TObjectIterator<APlatformBox> act; act; ++act) //find all platforms currently active in scene
		{
			if (act->IsPendingKill()) //if they aren't being destroyed
			{
				continue;
			}
			if (act->GetWorld() != this->GetWorld()) //and they currently exist in the active scene
			{
				continue;
			}
			if (act->GetType() == 1) //if platform is a destructable box
			{
				platformCount++; //add one to platform count as not all platforms are destroyed
			}
		}
		
		if (platformCount <= 0) //if platform count is still 0 after searching for platforms
		{
			level++; //increment level

			//Reset any values that need resetting for next level
			multiplier = 1.0f;
			bMainBall = false;
			bMultiplierCanEarnBall = true;
			bonusBallString = "";
			timerCount = 0.0f;
			bTimerActive = false;
			platformCount = NUMBOXWIDTH * NUMBOXHEIGHT;
			if (level != 1) //level 1 has tutorial for controls so shouldn't be overridden
			{
				tutorialText = "Level " + FString::FromInt(level); //set the start of level text to display the level name
			}
			DestroyBalls(); //destroy any balls currently active in the scene
			DestroyPlatforms(); //destroy moving platform

			SpawnPlatforms(level); //respawn the platforms using level to determine platform health
		}
	}
}

//function to destroy all active balls in the scene
void AMyPawn::DestroyBalls() 
{
	// Find all the balls currently active in the scene
	for (TObjectIterator<ABullet> act; act; ++act)
	{
		if (act->IsPendingKill()) //if they aren't already being destroyed
		{
			continue;
		}
		if (act->GetWorld() != this->GetWorld()) //and if they are active in the scene
		{
			continue;
		}
		if (act->ballType == 1) //if it is a main ball left in the scene after the level is completed
		{
			if (ballNum < 3) //and the number of player's lives is not full
			{
				ballNum++; //add a life on for completing the level
			}
		}
		act->Destroy(); //destroy all balls ready for next level
	}

}

//function to destroy all active platforms in the scene except type 0 platforms (walls)
void AMyPawn::DestroyPlatforms()
{
	// Find all platforms currently active in the scene
	for (TObjectIterator<APlatformBox> act; act; ++act)
	{
		if (act->GetWorld() != this->GetWorld()) //if they are active in the scene
		{
			continue;
		}
		if (act->GetType() >= 1) //if they are destructible or are the moving platform (type 1 or 2, and not the walls and roof)
		{
			act->Destroy(); //then destroy the platform
		}
	}
}

//function called to re-spawn all platforms for level
void AMyPawn::SpawnPlatforms(int level) 
{
	UMaterialInterface* pointBoxMat; //create a temporary material variable that will be assigned to the platform based on its health value
	pointBoxMat = platformNormalMaterial; //give the material variable a default value
	switch (level) //depending on level assign speed of ball, maximum platform health, number of starting lives and spawn a moving platform
	{
	case 1: ballSpeed = 5.0f; platformHealth = 1; break;
	case 2: ballSpeed = 6.0f; platformHealth = 2; break;
	case 3: ballSpeed = 7.0f; platformHealth = 3; SpawnMovingPlatform(1.0f); break;
	case 4: ballSpeed = 8.0f; platformHealth = 4; SpawnMovingPlatform(1.5f); break;
	case 5: ballSpeed = 9.0f; platformHealth = 5; SpawnMovingPlatform(2.0f); break;
	case 6: ballSpeed = 10.0f; platformHealth = 5; ballNum = 2; SpawnMovingPlatform(2.5f); break;
	case 7: ballSpeed = 10.0f; platformHealth = 5; ballNum = 1; SpawnMovingPlatform(3.0f); break;
	default: ballSpeed = (level + 3.0f); platformHealth = 5; ballNum = 1; SpawnMovingPlatform(3.0f); break; //default values set for levels after level 7
	}
	//spawn destructible boxes in world
	APlatformBox *PointBox = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass());

	//nested array to create a grid of platforms with specified width and height
	for (int i = 0; i < NUMBOXWIDTH; i++) 
	{
		for (int j = 0; j < NUMBOXHEIGHT; j++)
		{
			int tempPlatformHealth = FMath::RandRange(1, platformHealth); //choose a random value for platform health between 1 and the maximum health for that level
			
			//assign a material to the platform based on the random health it is assigned
			switch (tempPlatformHealth)
			{
			case 1: pointBoxMat = platform1Material; break;
			case 2: pointBoxMat = platform2Material; break;
			case 3: pointBoxMat = platform3Material; break;
			case 4: pointBoxMat = platform4Material; break;
			case 5: pointBoxMat = platformNormalMaterial; break;
			default: pointBoxMat = platformNormalMaterial; break;
			}

			PointBox = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass()); //create a platform box actor in the scene
			PointBox->SetPosition(FVector(0.0f, (i*BOX_PADDING_WIDTH) - (LEVELWIDTH - 537.5f), (j*BOX_PADDING_HEIGHT) + 2000)); //set platform position including padding for width and height (extra amount taken off/added to start grid in correct position)
			PointBox->SetScale(FVector(2.5f, 4.0f, 2.0f)); //sets the scale of the platform box
			PointBox->SetType(1); //sets the type of the box to a destructible platform
			PointBox->SetHealth(tempPlatformHealth); //assigns the health value to the box
			PointBox->SetMyMaterial(pointBoxMat); //assigns the material to the platform box
			PointBox->UpdateBounds(); //updates the bounds of the box based on its new location
		}
	}
}

//function to spawn a moving platform in the level
void AMyPawn::SpawnMovingPlatform(float speed) //takes a speed parameter to increase difficulty of avoiding platform
{
	APlatformBox *MovingPlatform = (APlatformBox*)GWorld->SpawnActor(APlatformBox::StaticClass()); //create a platform box actor in the scene
	MovingPlatform->SetPosition(FVector(0.0f, 0.0f, 1500.0f)); //set the position of the box to be just below the destructible platforms
	MovingPlatform->SetScale(FVector(2.5f, 8.0f, 1.0f)); //set the scale of the box
	MovingPlatform->SetType(2); //set the type of the platform to a moving platform
	MovingPlatform->SetHealth(1); //give the platform a basic value for health as type makes it indestructible
	MovingPlatform->SetSpeed(speed); //assign the speed of the platform
	MovingPlatform->SetMyMaterial(movingPlatformMaterial); //assign a material to the platform from the inspector
	MovingPlatform->UpdateBounds(); //update the bounds of the platform based on its new location
}

//function called when a ball is de-spawned and ballNum is 0
void AMyPawn::OutOfBalls()
{
	numBallsRemaining = 0; //reset the variable used to check how many balls are left in the scene
	if (ballNum <= 0) //if number of balls (lives) = 0
	{
		for (TObjectIterator<ABullet> act; act; ++act) //find all balls currently active in scene
		{
			if (act->IsPendingKill())
			{
				continue;
			}
			if (act->GetWorld() != this->GetWorld())
			{
				continue;
			}
			numBallsRemaining++; //for each ball that exists, add one to the count of balls remaining
		}

		if (numBallsRemaining <= 0) //if number of balls remaining is still 0 after search has finished
		{
			GameOver(); //display end of game UI and reset level
		}
	}
}

//function for end of game to display end of game UI and reset game
void AMyPawn::GameOver() 
{
	//Reset any values that need resetting for next level
	multiplier = 1.0f;
	score = 0;
	bMainBall = false;
	bonusBallString = "";
	timerCount = 0.0f;
	bTimerActive = false;
	platformCount = NUMBOXWIDTH * NUMBOXHEIGHT;

	DestroyBalls(); //destroy any balls currently active in the scene
	ballNum = 3; //reset ballNum
	if (level == 1) //if the player dies on level one
	{
		tutorialText = "Unlucky! Press 'E' or 'Space' to try again"; //display encouragement
	}
	else //for death on any other level
	{
		tutorialText = "You made it " + FString::FromInt(level) + " levels! \nPress 'E' or 'Space' to play again"; //display level reached and instructions to start again
	}
	gameOverText = "Game Over!"; //display game over UI text

	level = 1; //reset level
	DestroyPlatforms(); //destroy any platforms currently active in scene
	SpawnPlatforms(level); //respawn the platforms using level to determine platform health
}