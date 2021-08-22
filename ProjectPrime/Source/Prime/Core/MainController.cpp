// Copyright 2021 Master Kenth

#include "MainController.h"

#include "Components/InputComponent.h"
#include "MainCharacter.h"

void AMainController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	AMainCharacter* pawnAsMainCharacter = Cast<AMainCharacter>(aPawn);
	if (pawnAsMainCharacter)
	{
		MainCharacter = pawnAsMainCharacter;
	}
}

void AMainController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis(TEXT("LookUp"), this, &AMainController::OnLookUp);
	InputComponent->BindAxis(TEXT("LookRight"), this, &AMainController::OnLookRight);
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AMainController::OnMoveForward);
	InputComponent->BindAxis(TEXT("MoveRight"), this, &AMainController::OnMoveRight);
}

void AMainController::OnLookUp(float Val)
{
	AddPitchInput(Val);
}

void AMainController::OnLookRight(float Val)
{
	AddYawInput(Val);
}

void AMainController::OnMoveForward(float Val)
{
	if (MainCharacter)
	{
		MainCharacter->AddMovementInput(MainCharacter->GetActorForwardVector() * Val);
	}
}

void AMainController::OnMoveRight(float Val)
{
	if (MainCharacter)
	{
		MainCharacter->AddMovementInput(MainCharacter->GetActorRightVector() * Val);
	}
}
