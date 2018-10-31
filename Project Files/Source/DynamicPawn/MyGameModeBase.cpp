// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicPawn.h"
#include "Blueprint/UserWidget.h"
#include "MyGameModeBase.h"

void AMyGameModeBase::BeginPlay() 
{
	Super::BeginPlay();

	////////////////////////////////////// Start of code adapted from Ward, 2016 ////////////////////////////////////////////

	if (PlayerHUDClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), PlayerHUDClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

}

AMyGameModeBase::AMyGameModeBase()
{

}

////////////////////////////////////// End of code adapted from Ward, 2016 ////////////////////////////////////////////


