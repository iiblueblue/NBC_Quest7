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

	// �߷� ����
	if (!bIsDroneActive)
	{
		if (!bIsGrounded)
		{
			Velocity.Z += Gravity * DeltaTime;
		}
	}

	FHitResult Hit;
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLocation, true, &Hit);

	if (Hit.bBlockingHit)
	{
		bIsGrounded = true;
		Velocity.Z = 0.0f;
		if (!bIsDroneActive)
		{
			SetActorLocation(Hit.Location + FVector(0.0f, 0.0f, 8.0f)); // ��Ȯ�� �浹 �������� �̵�
		}
	}
	else
	{
		bIsGrounded = false;
	}

	CheckGroundCollision();
	

	if (SkeletalMeshComp)
	{
		// �̵� ���� �ƴ� �� (bIsRecovering = true) �����Ͽ� ����
		if (bIsRecovering || !FMath::IsNearlyEqual(CurrentPitch, TargetPitch, 0.1f) || !FMath::IsNearlyEqual(CurrentRoll, TargetRoll, 0.1f))
		{
			// �ε巴�� Lerp ���� ����
			CurrentPitch = FMath::Lerp(CurrentPitch, TargetPitch, 0.1f);
			CurrentRoll = FMath::Lerp(CurrentRoll, TargetRoll, 0.1f);

			SkeletalMeshComp->SetRelativeRotation(FRotator(CurrentPitch, 0.0f, CurrentRoll));

			// ��ǥ���� ���� ��ġ�ϸ� ���� ����
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
		if (bIsDroneActive)
		{
			AddActorLocalOffset(FVector(MoveInput.X, 0.0f, 0.0f) * MoveSpeed);
			// ������ ����̱�
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
		else
		{
			AddActorLocalOffset(FVector(MoveInput.X, 0.0f, 0.0f) * (MoveSpeed * 0.3));
		}
		
	}
}

void ADrone::MoveRight(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector3d MoveInput = value.Get<FVector3d>();

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		if (bIsDroneActive)
		{
			AddActorLocalOffset(FVector(0.0f, MoveInput.Y, 0.0f) * MoveSpeed);

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
		else
		{
			AddActorLocalOffset(FVector(0.0f, MoveInput.Y, 0.0f) * (MoveSpeed * 0.3));
		}
	}
}

void ADrone::MoveUp(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector3d MoveInput = value.Get<FVector3d>();

	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		if (bIsDroneActive)
		{
			//AddActorLocalOffset(FVector(0.0f, 0.0f, MoveInput.Z)*MoveSpeed);
			FVector NewLocation = GetActorLocation() + FVector(0.0f, 0.0f, MoveInput.Z) * MoveSpeed;
			// �ٴ� �浹 ����
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			FVector Start = GetActorLocation();
			FVector End = Start + FVector(0.0f, 0.0f, MoveInput.Z * MoveSpeed);

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);

			if (bHit && MoveInput.Z < 0) // �ٴڰ� �浹������ �Ʒ��� �̵� ���̶��
			{
				bIsGrounded = true;
				Velocity.Z = 0.0f;
				SetActorLocation(HitResult.Location + FVector(0.0f, 0.0f, 8.0f)); // ��¦ ���� ����
			}
			else
			{
				bIsGrounded = false;
				SetActorLocation(NewLocation, true); // Sweep �����Ͽ� �ڿ������� �浹 ����
			}
		}
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
	AddActorLocalRotation(FRotator(0.0f, LookInput.X, 0.0f)); // ���� ȸ��

	// ī�޶� ȸ��(Y��)
	// ���� SpringArm�� ����� pitch ���� ������
	FRotator CurrentRotation = SpringArmComp->GetRelativeRotation();
	float NewPitch = CurrentRotation.Pitch + LookInput.Y;

	// Ư�� ���� ������ ���� (��: -45�� ~ 45��)
	NewPitch = FMath::Clamp(NewPitch, -45.0f, 45.0f);

	// ���ѵ� ���� ����
	SpringArmComp->SetRelativeRotation(FRotator(NewPitch, 0.0f, 0.0f));
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
	FVector End = Start + FVector(0.0f, 0.0f, -50.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // �ڱ� �ڽ��� ����

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);

	if (bHit)
	{
		bIsGrounded = true;
		Velocity.Z = 0.0f;
		if (!bIsDroneActive)
		{
			SetActorLocation(HitResult.Location+FVector(0.0f, 0.0f, 8.0f)); // ��Ȯ�� �浹 ��ġ ����
		}
	}
	else
	{
		bIsGrounded = false;
	}
}
