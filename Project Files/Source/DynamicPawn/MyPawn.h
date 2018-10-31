// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "PlatformBox.h"
#include "MyPawn.generated.h"

#define ACTORWIDTH (420.0f)
#define MOVESPEED (3.0f)
#define LEVELWIDTH (1975.0f)

#define NUMBOXWIDTH (6)
#define NUMBOXHEIGHT (5)
#define BOX_PADDING_HEIGHT (350)
#define BOX_PADDING_WIDTH (575)

class ABullet;

UCLASS()
class DYNAMICPAWN_API AMyPawn : public APawn
{
	GENERATED_BODY()

private:
	ACameraActor* OurCamera; //required to set the position of camera in begin play function
	APlayerCameraManager* CameraManager; //used to set target viewport of static camera

	float currentDeltaTime;	// Delta time needs to be accessible throughout the class

	float ballSpeed; //speed of the ball, changes per level

	float multiplier; //combo value increased by destroying boxes
	bool bMultiplierCanEarnBall; //bool used to stop player earning too many balls from one combo chain
	bool bTimerActive; //bool used to check if countdown is in progress
	float timerCount; //float used to display text for a certain amount of time
	FString bonusBallString; //text used to toggle bonus ball text on and off
	FString tutorialText; //text used to toggle tutorial/level name on and off
	FString gameOverText; //text used to toggle game over text on and off

	int score; //score variable

public:
	// Sets default values for this pawn's properties
	AMyPawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(EditAnywhere)
	USceneComponent* OurVisibleComponent;

	//function called to increase multiplier variable
	void IncrementMultiplier();
	//function called to increase the score variable by 10 * multiplier value
	void IncrementScore() { score += (10 * multiplier); }
	//function called when a type 1 platform (destructible) is hit
	void DecreasePlatformCount();
	//function to destroy all active balls in the scene
	void DestroyBalls();
	//function to destroy all active platforms in the scene except type 0 platforms (walls)
	void DestroyPlatforms();
	//function called to re-spawn all platforms for level
	void SpawnPlatforms(int level); 
	//function to spawn a moving platform in the level
	void SpawnMovingPlatform(float speed); 	
	//function called when a ball is de-spawned and ballNum is 0
	void OutOfBalls();
	//function for end of game to display end of game UI and reset game
	void GameOver();


	//Accessor functions for the multiplier variable
	void ResetMultiplier() { multiplier = 1.0f; bMultiplierCanEarnBall = true; }
	float GetMultiplier() { return multiplier; }

	//Input functions
	void Move_YAxis(float AxisValue);
	//function to spawn a main ball when 'E', 'Space' or 'up arrow' are pressed
	void SpawnMainBall(); 
	//function to spawn a regular bonus ball
	void SpawnBall(); 
	 //function to spawn multi-balls on the mega bonus
	void SpawnFanBalls();

	//Input variables
	FVector CurrentVelocity;
	FVector MoveVelocity;

	float ballNum; //number of balls available to fire (lives)
	bool bMainBall; //bool to keep track of whether a main ball exists in the scene
	int platformCount; //number of platforms remaining in the scene
	int platformHealth; //maximum health each platform can have in a level
	int level; //level number player has reached, starts at 1
	int numBallsRemaining; //a count of the number of active balls in the scene
	
	//Public in inspector material variables to allow easy assigning
	UPROPERTY(EditAnywhere)
		UMaterialInterface *mainBallMaterial;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *platformNormalMaterial;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *platform4Material;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *platform3Material;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *platform2Material;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *platform1Material;

	UPROPERTY(EditAnywhere)
		UMaterialInterface *movingPlatformMaterial;
	////////////////////////////////////// Start of code adapted from Ward, 2016 ////////////////////////////////////////////

	//UFunctions used in blueprints to access data and display it as UI
	UFUNCTION(BlueprintPure, Category = "Multiplier")
		float GetMultiplierFloat() { return multiplier; }

	////////////////////////////////////// End of code adapted from Ward, 2016 ////////////////////////////////////////////


	UFUNCTION(BlueprintPure, Category = "Ball Count")
		int GetBallNum() { return ballNum; }

	UFUNCTION(BlueprintPure, Category = "Bonus Ball")
		FString GetBonusBallString() { return bonusBallString; }

	UFUNCTION(BlueprintPure, Category = "Tutorial Text")
		FString GetTutorialText() { return tutorialText; }

	UFUNCTION(BlueprintPure, Category = "Game Over Text")
		FString GetGameOverText() { return gameOverText; }

	UFUNCTION(BlueprintPure, Category = "Score")
		int GetScore() { return score; }
};
