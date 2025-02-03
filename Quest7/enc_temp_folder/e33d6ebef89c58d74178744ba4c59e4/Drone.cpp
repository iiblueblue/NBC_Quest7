// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone.h"
#include "QuestPlayerController.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ADrone::ADrone()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component(Collision)"));
	SetRootComponent(SphereComp);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComp->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SkeletalMeshComp)
	{
		if (bIsRecovering)
		{
			UE_LOG(LogTemp, Warning, TEXT("Recovering!!!!!!!!!!!!!!"));
			CurrentPitch = FMath::Lerp(CurrentPitch, TargetPitch, 0.0f);
			CurrentRoll = FMath::Lerp(CurrentRoll, TargetRoll, 0.0f);
		}

		UE_LOG(LogTemp, Warning, TEXT("Lerp!!!!!!!!!!!!!!!"));
		SkeletalMeshComp->SetRelativeRotation(FRotator(CurrentPitch, 0.0f, CurrentRoll));

		if (FMath::IsNearlyZero(CurrentPitch, 0.1f) && FMath::IsNearlyZero(CurrentRoll, 0.1f))
		{
			UE_LOG(LogTemp, Warning, TEXT("Recovering End!!!!!!!!!!!!!!"));
			CurrentPitch = 0.0f;
			CurrentRoll = 0.0f;
			bIsRecovering = false;
		}
	}
}

// Called to bind functionality to input
void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AQuestPlayerController* PlayerController = Cast<AQuestPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::MoveForward
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Completed,
					this,
					&ADrone::StopMove
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::MoveRight
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::MoveUp
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ADrone::Look
				);
			}
		}
	}

}

void ADrone::MoveForward(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector3d MoveInput = value.Get<FVector3d>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddActorLocalOffset(FVector(MoveInput.X, 0.0f, 0.0f)*MoveSpeed);
		// 앞으로 기울이기
		if (MoveInput.X > 0)
		{
			TargetPitch = -MoveTilt;
		}
		else
		{
			TargetPitch = MoveTilt;
		}
		bIsRecovering = false;
	}
}

void ADrone::MoveRight(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector3d MoveInput = value.Get<FVector3d>();

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddActorLocalOffset(FVector(0.0f, MoveInput.Y, 0.0f)*MoveSpeed);
	}
}

void ADrone::MoveUp(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector3d MoveInput = value.Get<FVector3d>();

	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		AddActorLocalOffset(FVector(0.0f, 0.0f, MoveInput.Z)*MoveSpeed);
	}
}

void ADrone::StopMove(const FInputActionValue& value)
{
	TargetPitch = 0.0f;
	TargetRoll = 0.0f;
	bIsRecovering = true;
}

void ADrone::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	
	AddActorLocalRotation(FRotator(0.0f, LookInput.X, 0.0f)); // 액터 회전

}