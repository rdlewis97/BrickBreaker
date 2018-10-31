// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicPawn.h"
#include "Bullet.h"

// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	SphereVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Sphere.sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		SphereVisual->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
		SphereVisual->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));
	}
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	//find the player paddle
	for (TObjectIterator<AMyPawn> act; act; ++act)
	{

		if (act->GetWorld() != this->GetWorld())
		{
			continue;
		}
		playerPaddle = *act; //assign the actor instance to the player paddle variable for use accessing script later on
	}

	//initialise variables
	closestPlatform = NULL;
	platformUpdateCounter = 0;
	collisionFlags = 0;
	platformHealth = playerPaddle->platformHealth;
	ballSpeed = 0;
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Process velocity to move the bullet
	if (!velocity.IsZero())
	{
		FVector NewLocation = GetActorLocation() + (velocity * (DeltaTime * 3));

		//check if ball has hit edges of level
		if ((NewLocation.Y*NewLocation.Y) >= (LEVELWIDTH-(BALLWIDTH))*(LEVELWIDTH-(BALLWIDTH)))
		{
			NewLocation.Y = GetActorLocation().Y; //stop ball moving
			velocity.Y = velocity.Y * -1.0f; //invert its velocity to make it bounce off
		}
		if (NewLocation.Z >= (LEVELWIDTH * 2.0f) - BALLWIDTH) //if ball reaches roof then bounce
		{
			velocity.Z = velocity.Z * -1.0f;
		}
		if (NewLocation.Z < 0.0f) //if ball goes past paddle
		{
			playerPaddle->ResetMultiplier(); //reset the multiplier variable
			if (ballType == 1) //if it was a main ball
			{
				playerPaddle->bMainBall = false; //set bool to false to indicate no active main ball in scene
				//means that player can fire another main ball
			}
			
			Destroy(); //destroy the ball

			if (playerPaddle->ballNum <= 0) //if the player has no balls left to spawn
			{
				//perform check for number of balls active in scene 
				playerPaddle->OutOfBalls();
			}
		}
		//check for bounce off paddle
		if (NewLocation.Z <= 150.0f && NewLocation.Z > 40.0f)
		{
			//update player to get latest location
			for (TObjectIterator<AMyPawn> act; act; ++act)
			{
				if (act->GetWorld() != this->GetWorld())
				{
					continue;
				}
				//compare distance between paddle and ball to see if deflection is necessary
				float actorLocation = act->GetActorLocation().Y;
				float distance = actorLocation - NewLocation.Y;
				if ((distance*distance) <= (ACTORWIDTH*ACTORWIDTH))
				{
						if (distance < 0) //if ball is right side of paddle
						{
							if (distance < -(ACTORWIDTH / 2)) //if ball hits far right quarter of paddle
							{
								velocity.Y += 300.0f; //affect bounce by adding force to right
								act->CurrentVelocity.Y = 0.0f; //reset currentVelocity to 0
							}
						}
						else //if ball is left side of paddle
						{
							if (distance > ACTORWIDTH/2) //if ball hits far left quarter of paddle
							{
								velocity.Y -= 300.0f; //affect bounce by adding force to left
								act->CurrentVelocity.Y = 0.0f; //reset current velocity to 0
							}
						}
					
					velocity.Y += act->CurrentVelocity.Y; //affect the bullet direction by the current velocity of the paddle

					NewLocation.Z = GetActorLocation().Z; //stop ball moving through paddle
					velocity.Z = velocity.Z * -1.0f; //invert balls vertical velocity
					if (ballType == 1) //if ball is a main ball
					{
						playerPaddle->ResetMultiplier(); //reset the multiplier variable
					}
				}
			}
		}

		if (closestPlatform) //if a closest platform has been found
		{
			// Get the new collision flags and first check we are going to enter the box
			int checkFlags = APlatformBox::X_AXIS | APlatformBox::Y_AXIS | APlatformBox::Z_AXIS;
			int newCollisionFlags = closestPlatform->IsInBox(NewLocation, (BALLWIDTH-10), (BALLWIDTH/2));
			if (newCollisionFlags == checkFlags)
			{
				// Find out which flag(s) changed to put us in the box, and act on the NewLocation value accordingly
				int changedFlags = newCollisionFlags - collisionFlags;

				//reduce platform health by 1
				if (closestPlatform->GetType() == 1) //if platform is a destructible platform
				{
					platformHealth = closestPlatform->GetHealth(); //return its health
					platformHealth--; //reduce the health by one 
					UMaterialInterface* boxMat; //create a temporary variable to store the new material
					
					switch (platformHealth) //based on the health of the platform, assign the correct material to show damage
					{
					case 1: boxMat = playerPaddle->platform1Material; break;
					case 2: boxMat = playerPaddle->platform2Material; break;
					case 3: boxMat = playerPaddle->platform3Material; break;
					case 4: boxMat = playerPaddle->platform4Material; break;
					default: boxMat = playerPaddle->platformNormalMaterial; break;
					}
					closestPlatform->SetMyMaterial(boxMat); //use the accessor function to set the material of the platform

					closestPlatform->SetHealth(platformHealth); //set the new reduced health of the platform

					playerPaddle->IncrementMultiplier(); //increment the multiplier variable

					if (platformHealth <= 0) //if the platform health is 0 or less
					{
						closestPlatform->Destroy(); //destroy the platform
						playerPaddle->DecreasePlatformCount(); //call the function to reduce platform count on the player paddle
						//this also performs the check to see if there are any active balls left in the scene
					}
				}
				//handle bounces based on which face of the platform is hit
				if (changedFlags & APlatformBox::X_AXIS)
				{
					velocity.X = velocity.X * -1.0f; 
				}

				if (changedFlags & APlatformBox::Y_AXIS)
				{
					velocity.Y = velocity.Y * -1.0f;
				}

				if (changedFlags & APlatformBox::Z_AXIS_POS)
				{
					// Collision with the top side
					velocity.Z = velocity.Z * -1.0f;
				}

				if (changedFlags & APlatformBox::Z_AXIS_NEG)
				{
					// Collision with the underside
					velocity.Z = velocity.Z * -1.0f;
				}
			}
		}
		//clamp to moderate the speed of the ball when it is hit at an angle repeatedly
		//can never get larger than 2 times the speed it started at
		velocity.Z = FMath::Clamp(velocity.Z, -(2*ballSpeed), (2 * ballSpeed));
		velocity.Y = FMath::Clamp(velocity.Y, -(2 * ballSpeed), (2 * ballSpeed));

		SetActorLocation(NewLocation);

		// Once we have completed all checks, we need to record the current flags for next frame
		if (closestPlatform)
		{
			collisionFlags = closestPlatform->IsInBox(NewLocation, (BALLWIDTH-10), (BALLWIDTH/2));
		}
		else
		{
			collisionFlags = 0;
		}
	}

	// Update the closest platform variable. Used to limit the number of times the closest platform check is called
	if (platformUpdateCounter-- <= 0)
	{
		closestPlatform = FindClosestPlatform();
		platformUpdateCounter = 1;
	}
}

// Calculate a velocity that will get us there based off the direction and velocity passed in
void ABullet::CalculateVelocity(float ballVelocity, FVector &ballDirection)
{
	velocity = ballDirection;
	velocity.Normalize();
	velocity *= ballVelocity;
	ballSpeed = ballVelocity; //store a reference to the initial speed for moderation
}

// This function performs a search for the closest platform to the ball
APlatformBox* ABullet::FindClosestPlatform()
{
	APlatformBox *closestPlatform = NULL;
	float closestDistanceSqr = (float)1e10;	// Very big number!

	// Find all active platform boxes in the scene
	for (TObjectIterator<APlatformBox> act; act; ++act)
	{
		if (act->IsPendingKill())
		{
			continue;
		}
		if (act->GetWorld() != this->GetWorld())
		{
			continue;
		}
		if (act->GetType() >= 1) //if they are not the walls or roof
		{
			FVector distanceToTarget = (act->GetActorLocation() - this->GetActorLocation()); //calculate the distance to the platform
			distanceToTarget.X = 0.0f; //set the X value to 0 as it shouldnt be taken into account
			if (distanceToTarget.SizeSquared() < closestDistanceSqr) //if it is closer than any other platform
			{
				closestDistanceSqr = distanceToTarget.SizeSquared(); //update new closest distance
				closestPlatform = *act; //update closest platform actor
			}
		}
	}

	return closestPlatform; //return the actor which was the closest to the ball
}