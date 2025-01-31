// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestGameMode.h"
#include "QuestPlayerController.h"
#include "Drone.h"

AQuestGameMode::AQuestGameMode()
{
	DefaultPawnClass = ADrone::StaticClass();
	PlayerControllerClass = AQuestGameMode::StaticClass();
}
