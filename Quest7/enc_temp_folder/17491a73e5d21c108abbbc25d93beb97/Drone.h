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
	// ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* CameraComp;

	// �Ϲ� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MoveSpeed = 30.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MoveTilt = 10.0f;
	float CurrentPitch = 0.0f;
	float CurrentRoll = 0.0f;
	float TargetPitch = 0.0f;
	float TargetRoll = 0.0f;
	bool bIsRecovering = true;

	// ����������Ŭ �Լ�
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// �Է� ���ε� �Լ�
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// �Ϲ� �Լ�
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
};
