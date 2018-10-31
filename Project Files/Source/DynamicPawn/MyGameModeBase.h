// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DYNAMICPAWN_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

		virtual void BeginPlay() override;
	////////////////////////////////////// Start of code adapted from Ward, 2016 ////////////////////////////////////////////
public:
	AMyGameModeBase();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Multiplier", Meta = (BlueprintProtected = "true"))
		TSubclassOf<class UUserWidget> PlayerHUDClass;

	UPROPERTY()
		class UUserWidget* CurrentWidget;
	////////////////////////////////////// End of code adapted from Ward, 2016 ////////////////////////////////////////////

};
