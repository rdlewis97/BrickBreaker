// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformBox.generated.h"

#define LEVELWIDTH (1975.0f)

UCLASS()
class DYNAMICPAWN_API APlatformBox : public AActor
{
	GENERATED_BODY()

private:
	UStaticMeshComponent* CubeVisual;
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;

	int type; // 0: normal wall, 1: collectible box, 2: moving platform
	int health;
	FVector currentVelocity;
	float platformSpeed;

public:	

	enum collisionFlags
	{
		X_AXIS = 1,
		Y_AXIS = 2,
		Z_AXIS_POS = 4,
		Z_AXIS_NEG = 8,
		Z_AXIS = 12,
	};

	// Sets default values for this actor's properties
	APlatformBox();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int IsInBox(FVector testVector, float radius, float height = 0.0f);
	void UpdateBounds();
	//accessor functions to setup the platform boxes from other scripts
	void SetPosition(FVector pos) { SetActorLocation(pos); UpdateBounds(); }
	void SetScale(FVector scale) { SetActorScale3D(scale); }
	void SetRotation(FRotator rotation) { SetActorRotation(rotation); }
	//type is used to determine if the platform box is destructible or not
	void SetType(int myType) { type = myType; } //0: wall or roof, 1: destructible box, 2: moving platform
	int GetType() { return type; }
	//used for setting the health of the platform after a collision has occurred
	void SetHealth(int myHealth) { health = myHealth; }
	int GetHealth() { return health; }
	//used to find the top of the box for collisions
	float GetMaxZ() { return maxZ; }
	//used for moving platforms to give them a speed based on the level
	void SetSpeed(float speed) { platformSpeed = speed; }
	//used to set the material of the platform box from other scripts
	void SetMyMaterial(UMaterialInterface* material) { CubeVisual->SetMaterial(0, material); }

};
