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
	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* MainCollider;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	virtual void BeginPlay() override;
};
