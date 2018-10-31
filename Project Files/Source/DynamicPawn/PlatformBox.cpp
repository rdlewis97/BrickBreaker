// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicPawn.h"
#include "PlatformBox.h"


// Sets default values
APlatformBox::APlatformBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubeVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	CubeVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeVisualAsset(TEXT("/Engine/BasicShapes/Cube.cube"));
	if (CubeVisualAsset.Succeeded())
	{
		CubeVisual->SetStaticMesh(CubeVisualAsset.Object);
		CubeVisual->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));
		CubeVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	}
}

// Called when the game starts or when spawned
void APlatformBox::BeginPlay()
{
	Super::BeginPlay();
	//initialise variables
	type = 0;
	platformSpeed = 1.0f;
	if (health == NULL)
	{
		health = 1;
	}
}

// Called every frame
void APlatformBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (type == 2) //moving platform
	{
		if (currentVelocity.Y == 0.0f && GetActorLocation().Y == 0.0f) //if it hasnt moved yet and is stationary then give it a velocity
		{
			currentVelocity = FVector(0.0f, 5.0f, 0.0f);
		}
		if (GetActorLocation().Y >= (LEVELWIDTH - 725.0f) || GetActorLocation().Y <= -(LEVELWIDTH - 725.0f)) //limit the movement of the platform
		{
			currentVelocity.Y *= -1.0f; //invert horizontal velocity if platform reaches edge of limit
		}

		FVector NewLocation = GetActorLocation() + (currentVelocity * DeltaTime * 100.0f * platformSpeed);
		
		SetActorLocation(NewLocation);
		UpdateBounds(); // used to ensure bounds of box are updated based on new location
	}

}

//function to check which flags are triggered in collision
int APlatformBox::IsInBox(FVector testVector, float radius, float height)
{
	int returnFlags = 0;

	if (((testVector.X + radius) >= minX) && ((testVector.X - radius) <= maxX))
	{
		returnFlags |= X_AXIS;
	}

	if (((testVector.Y + radius) >= minY) && ((testVector.Y - radius) <= maxY))
	{
		returnFlags |= Y_AXIS;
	}

	if ((testVector.Z) + height >= minZ)
	{
		returnFlags |= Z_AXIS_NEG;		// We have crossed over the lower Z_AXIS boundary, hence the _NEG flag.
	}

	if ((testVector.Z) < maxZ)
	{
		returnFlags |= Z_AXIS_POS;		// and the reverse - the lower extreme of the test object is over the upper Z_AXIS boundary
	}

	return returnFlags;
}

//function used to update the bounds of the box for collisions based on its location
void APlatformBox::UpdateBounds()
{
	FVector origin, bounds;
	GetActorBounds(true, origin, bounds);

	minX = origin.X - bounds.X;
	maxX = origin.X + bounds.X;
	minY = origin.Y - bounds.Y;
	maxY = origin.Y + bounds.Y;
	minZ = origin.Z - bounds.Z;
	maxZ = origin.Z + bounds.Z;
}

