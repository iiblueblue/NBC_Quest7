// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestPlayerController.h"
#include "EnhancedInputSubsystems.h"

AQuestPlayerController::AQuestPlayerController()
	:InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	OnAction(nullptr)
{
}

void AQuestPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
