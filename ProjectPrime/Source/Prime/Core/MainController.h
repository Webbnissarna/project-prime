// Copyright 2021 Master Kenth

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MainController.generated.h"

/**
 *
 */
UCLASS()
class PRIME_API AMainController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	class AMainCharacter* MainCharacter;

	virtual void OnPossess(APawn* aPawn) override;
	virtual void SetupInputComponent() override;

	void OnLookUp(float Val);
	void OnLookRight(float Val);
	void OnMoveForward(float Val);
	void OnMoveRight(float Val);
};
