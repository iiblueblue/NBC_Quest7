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

	// Sphere Component(Root Component)
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component(Collision)"));
	SetRootComponent(SphereComp);

	// Skeletal Mesh Component
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshComp->SetupAttachment(RootComponent);

	// Static Mesh Component
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OnOff Light"));
	StaticMeshComp->SetupAttachment(SkeletalMeshComp);

	// Spring Arm Component
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	// Camera Component
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
	if (!bIsDroneActive) // 드론이 꺼져 있을 때
	{
		if (!bIsGrounded) // 공중에 있을 때
		{
			Velocity.Z += Gravity * DeltaTime; // Velocity = Velocity + (Gravity * DeltaTime)
		}
	}

	// 위치 업데이트
	FHitResult Hit;
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime; // Position = Position + (Velocity * DeltaTime)
	SetActorLocation(NewLocation, true, &Hit); // 위치 변경

	// 충돌 감지
	if (Hit.bBlockingHit) // 바닥과 충돌하였을 때
	{
		bIsGrounded = true; // 바닥에 있음 표시
		Velocity.Z = 0.0f; // 속도 0 처리
		if (!bIsDroneActive) // 드론이 꺼져 있다면
		{
			SetActorLocation(Hit.Location + FVector(0.0f, 0.0f, 8.0f)); // 정확한 충돌 지점으로 이동(위치 보정)
		}
	}
	else // 공중에 있을 때
	{
		bIsGrounded = false; // 바닥에 있지 않음 표시
	}

	CheckGroundCollision(); // 추가적인 바닥 충돌 검사
	

	// 이동 방향에 따라 드론 기울이고 복귀시키기
	if (SkeletalMeshComp)
	{
		// 이동 중이 아닐 때 (bIsRecovering = true) 보간하여 복귀
		if (bIsRecovering || !FMath::IsNearlyEqual(CurrentPitch, TargetPitch, 0.1f) || !FMath::IsNearlyEqual(CurrentRoll, TargetRoll, 0.1f))
		{
			// 부드럽게 Lerp 보간 적용
			CurrentPitch = FMath::Lerp(CurrentPitch, TargetPitch, 0.1f);
			CurrentRoll = FMath::Lerp(CurrentRoll, TargetRoll, 0.1f);

			SkeletalMeshComp->SetRelativeRotation(FRotator(CurrentPitch, 0.0f, CurrentRoll)); // Skeletal Mesh 회전

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
		if (bIsDroneActive)
		{
			AddActorLocalOffset(FVector(MoveInput.X, 0.0f, 0.0f) * MoveSpeed);
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
			// 바닥 충돌 감지
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			FVector Start = GetActorLocation();
			FVector End = Start + FVector(0.0f, 0.0f, MoveInput.Z * MoveSpeed);

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);

			if (bHit && MoveInput.Z < 0) // 바닥과 충돌했으며 아래로 이동 중이라면
			{
				bIsGrounded = true;
				Velocity.Z = 0.0f;
				SetActorLocation(HitResult.Location + FVector(0.0f, 0.0f, 8.0f)); // 살짝 위로 보정
			}
			else
			{
				bIsGrounded = false;
				SetActorLocation(NewLocation, true); // Sweep 적용하여 자연스러운 충돌 감지
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
	AddActorLocalRotation(FRotator(0.0f, LookInput.X, 0.0f)); // 액터 회전

	// 카메라 회전(Y축)
	// 현재 SpringArm의 상대적 pitch 값을 가져옴
	FRotator CurrentRotation = SpringArmComp->GetRelativeRotation();
	float NewPitch = CurrentRotation.Pitch + LookInput.Y;

	// 특정 각도 범위로 제한 (예: -45도 ~ 45도)
	NewPitch = FMath::Clamp(NewPitch, -45.0f, 45.0f);

	// 제한된 값을 적용
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
	Params.AddIgnoredActor(this); // 자기 자신은 제외

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);

	if (bHit)
	{
		bIsGrounded = true;
		Velocity.Z = 0.0f;
		if (!bIsDroneActive)
		{
			SetActorLocation(HitResult.Location+FVector(0.0f, 0.0f, 8.0f)); // 정확한 충돌 위치 보정
		}
	}
	else
	{
		bIsGrounded = false;
	}
}
