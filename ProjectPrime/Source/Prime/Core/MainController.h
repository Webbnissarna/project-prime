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
	virtual void SetupInputComponent() override;

	void OnLookUp(float Val);
	void OnLookRight(float Val);
};
