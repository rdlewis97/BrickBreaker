// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MyPawn.h"
#include "PlatformBox.h"
#include "Bullet.generated.h"

#define BALLWIDTH (100.0f)

UCLASS()
class DYNAMICPAWN_API ABullet : public AActor
{
	GENERATED_BODY()
	
private:
	FVector velocity; //velocity of the ball
	float ballSpeed; //stored float from player paddle script
	AMyPawn *playerPaddle; //stored instance of myPawn actor (player paddle)

	APlatformBox* closestPlatform; //stored instance of platform box actor (returned in FindClosestPlatform())
	int platformUpdateCounter; //a counter to reduce number of times search for closest platform is performed
	int collisionFlags; //flags created to monitor which side of platform is hit

	int platformHealth; //variable which stores closest platforms health

public:	
	// Sets default values for this actor's properties
	ABullet();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	//calculate an initial velocity for the ball
	void CalculateVelocity(float ballVelocity, FVector &ballDirection);

	//accessor function to set the material of the ball (used to highlight the main ball from bonus balls)
	void SetMyMaterial(UMaterialInterface* material) { SphereVisual->SetMaterial(0, material); }

	// Find the closest platform to this ball
	APlatformBox* FindClosestPlatform();

	int ballType; // 1: main ball, 2: bonus ball

	//public variable so value can be used at any point to set material
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent *SphereVisual;

};
