// Copyright 2021 Master Kenth

#include "MainController.h"

#include "Components/InputComponent.h"

void AMainController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis(TEXT("LookUp"), this, &AMainController::OnLookUp);
	InputComponent->BindAxis(TEXT("LookRight"), this, &AMainController::OnLookRight);
}

void AMainController::OnLookUp(float Val)
{
	AddPitchInput(Val);
}

void AMainController::OnLookRight(float Val)
{
	AddYawInput(Val);
}
