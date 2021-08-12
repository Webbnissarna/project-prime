// Copyright 2021 Master Kenth

#include "MainCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "MainCharacterMovementComponent.h"

AMainCharacter::AMainCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;

	MainMovement = CreateDefaultSubobject<UMainCharacterMovementComponent>(TEXT("MainMovement"));

	MainCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MainCollider"));
	SetRootComponent(MainCollider);
	MainCollider->SetCapsuleHalfHeight(175.0f / 2.0f);
	MainCollider->SetCapsuleRadius(30.0f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(MainCollider);
	Camera->SetRelativeLocation(FVector(0, 0, 65.0f));
	Camera->SetFieldOfView(90.0f);
	Camera->bUsePawnControlRotation = true;
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
}
