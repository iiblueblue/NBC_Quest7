// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Drone.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USphereComponent;
struct FInputActionValue;

UCLASS()
class QUEST7_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADrone();

protected:
	// Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	// Material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterial* MaterialOn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterial* MaterialOff;

	// 일반 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MoveSpeed = 30.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MoveTilt = 10.0f;
	float CurrentPitch = 0.0f;
	float CurrentRoll = 0.0f;
	float TargetPitch = 0.0f;
	float TargetRoll = 0.0f;
	bool bIsRecovering = true;
	bool bIsDroneActive = false;
	bool bIsGrounded = false;

	FVector3d Acceleration; // 가속도
	FVector3d Velocity; // 현재 속도
	const float Gravity = -980.0f; // 중력 가속도(cm/s^2)

	// 라이프사이클 함수
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 입력 바인딩 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 일반 함수
	UFUNCTION()
	void MoveForward(const FInputActionValue& value);
	UFUNCTION()
	void MoveRight(const FInputActionValue& value);
	UFUNCTION()
	void MoveUp(const FInputActionValue& value);
	UFUNCTION()
	void StopMove(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void OnDrone(const FInputActionValue& value);

	void CheckGroundCollision();
};
