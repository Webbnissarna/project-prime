// Copyright 2021 Master Kenth

#include "MainCharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/PhysicsVolume.h"

void UMainCharacterMovementComponent::TickComponent(
	float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent)
	{
		return;
	}

	const float MaxSpeed = 1200.0f;

	FVector oldVelocity = Velocity;
	FVector input = ConsumeInputVector();
	FVector scaledInput = input * MaxSpeed;

	Velocity = scaledInput;
	Velocity.Z = oldVelocity.Z + GetGravityZ() * DeltaTime;
	float terminalVelocity = GetPhysicsVolume()->TerminalVelocity;

	FVector Delta = Velocity * DeltaTime;
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
		Velocity.Z = 0;
	}

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Vel=%s"), *Velocity.ToString()));
}