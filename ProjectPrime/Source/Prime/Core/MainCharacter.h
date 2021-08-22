// Copyright 2021 Master Kenth

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "MainCharacter.generated.h"

UCLASS()
class PRIME_API AMainCharacter : public APawn
{
	GENERATED_BODY()

public:
	AMainCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere)
	class UMainCharacterMovementComponent* MainMovement;

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* MainCollider;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	virtual void BeginPlay() override;
};
