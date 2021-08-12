// Copyright 2021 Master Kenth

#include "MainCharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

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

	FVector input = ConsumeInputVector();

	FVector startLoc = PawnOwner->GetActorLocation() + FVector(0, 0, 50.0f);
	FVector endLoc = startLoc + input.GetSafeNormal() * 100.0f;
	DrawDebugLine(GetWorld(), startLoc, endLoc, FColor::Cyan, false, DeltaTime * 2);
	DrawDebugSphere(GetWorld(), endLoc, 20.0f, 8, FColor::Cyan, false, DeltaTime * 2);

	Velocity = input * MaxSpeed;

	FVector Delta = Velocity * DeltaTime;
	if (!Delta.IsNearlyZero())
	{
		const FQuat Rotation = UpdatedComponent->GetComponentQuat();

		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HandleImpact(Hit, DeltaTime, Delta);
			SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
		}
	}

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(
		-1, DeltaTime, FColor::Red, FString::Printf(TEXT("start=%s, end=%s"), *startLoc.ToString(), *endLoc.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Vel=%f"), Velocity.Size()));
}