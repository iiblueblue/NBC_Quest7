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

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OnOff Light"));
	StaticMeshComp->SetupAttachment(SkeletalMeshComp);

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

	// 중력 적용
	if (!bIsDroneActive)
	{
		if (!bIsGrounded)
		{
			Velocity.Z += Gravity * DeltaTime;
			SetActorLocation(GetActorLocation() + DeltaTime * Velocity);

			FVector CurrentLocation = GetActorLocation();
			if (CurrentLocation.Z <= 0.0f)
			{
				CurrentLocation.Z = 0.0f;
				Velocity.Z = 0.0f;
			}
			SetActorLocation(CurrentLocation);
		}
	}

	FHitResult Hit;
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLocation, true, &Hit);

	if (Hit.bBlockingHit)
	{
		bIsGrounded = true;
		Velocity.Z = 0.0f;
	}
	else
	{
		bIsGrounded = false;
	}

	CheckGroundCollision();
	

	if (SkeletalMeshComp)
	{
		// 이동 중이 아닐 때 (bIsRecovering = true) 보간하여 복귀
		if (bIsRecovering || !FMath::IsNearlyEqual(CurrentPitch, TargetPitch, 0.1f) || !FMath::IsNearlyEqual(CurrentRoll, TargetRoll, 0.1f))
		{
			// 부드럽게 Lerp 보간 적용
			CurrentPitch = FMath::Lerp(CurrentPitch, TargetPitch, 0.1f);
			CurrentRoll = FMath::Lerp(CurrentRoll, TargetRoll, 0.1f);

			SkeletalMeshComp->SetRelativeRotation(FRotator(CurrentPitch, 0.0f, CurrentRoll));

			// 목표값과 거의 일치하면 보간 종료
			if (FMath::IsNearlyEqual(CurrentPitch, TargetPitch, 0.1f) && FMath::IsNearlyEqual(CurrentRoll, TargetRoll, 0.1f))
			{
				bIsRecovering = false;
			}
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

				EnhancedInput->BindAction(
					PlayerController->OnAction,
					ETriggerEvent::Started,
					this,
					&ADrone::OnDrone
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

		if (MoveInput.Y > 0)
		{
			TargetRoll = MoveTilt;
		}
		else
		{
			TargetRoll = -MoveTilt;
		}

		bIsRecovering = false;
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
	SpringArmComp->AddRelativeRotation(FRotator(LookInput.Y, 0.0f, 0.0f));

}

void ADrone::OnDrone(const FInputActionValue& value)
{
	UE_LOG(LogTemp, Warning, TEXT("Drone onoff"));
	bIsDroneActive = !bIsDroneActive;
	if (bIsDroneActive)
	{
		StaticMeshComp->SetMaterial(0, MaterialOn);
	}
	else
	{
		StaticMeshComp->SetMaterial(0, MaterialOff);
	}
}

void ADrone::CheckGroundCollision()
{
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0.0f, 0.0f, -10.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 자기 자신은 제외

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		bIsGrounded = true;
		Velocity.Z = 0.0f;
	}
	else
	{
		bIsGrounded = false;
	}
}
