// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "QuestPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class QUEST7_API AQuestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AQuestPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;

	virtual void BeginPlay() override;
};
