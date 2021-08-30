// Copyright 2021 Master Kenth

#include "MainCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PhysicsVolume.h"

UMainCharacterMovementComponent::UMainCharacterMovementComponent()
	: MaxSpeed(1200.0f), UpdatedCollider(nullptr), MaxStepHeight(20), bIsOnGround(false)
{
}

void UMainCharacterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (UpdatedComponent)
	{
		UpdatedCollider = Cast<UCapsuleComponent>(UpdatedComponent);

		if (!UpdatedCollider)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find collider on UpdatedComponent for UMainCharacterMovementComponent"));
		}
	}
	else
	{
		UpdatedCollider = nullptr;
	}
}

void UMainCharacterMovementComponent::TickComponent(
	float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || !UpdatedCollider)
	{
		return;
	}

	FVector oldVelocity = Velocity;
	const FQuat rotation = UpdatedComponent->GetComponentQuat();
	float terminalVelocity = GetPhysicsVolume()->TerminalVelocity;
	float gravityZ = GetGravityZ();
	FVector input = ConsumeInputVector();
	FVector scaledInput = input * MaxSpeed;

	FVector desiredInputMovement = (scaledInput).GetClampedToMaxSize(MaxSpeed);
	FVector desiredMovement = desiredInputMovement;
	FVector desiredMovementDelta = desiredInputMovement * DeltaTime;

	/** Perform initial movement */
	FHitResult hit;
	SafeMoveUpdatedComponent(desiredMovementDelta, rotation, true, hit);

	if (hit.IsValidBlockingHit())
	{
		float surfaceDot = hit.Normal | FVector::UpVector;
		float surfaceAngleRad = FMath::Acos(surfaceDot);
		float surfaceAngle = FMath::RadiansToDegrees(surfaceAngleRad);

		FVector adjustedNormal = FVector(0, 0, 1).GetSafeNormal();

		FVector slideVec = ComputeSlideVector(desiredMovementDelta, 1.0f - hit.Time, adjustedNormal, hit);

		HandleImpact(hit, DeltaTime, desiredMovementDelta);
		SlideAlongSurface(desiredMovementDelta, 1.0f - hit.Time, adjustedNormal, hit, true);
	}

	/** Check for step down */
	if (MaxStepHeight > 0)
	{
		FVector groundTraceStartLocation = UpdatedCollider->GetComponentLocation();
		FHitResult groundHit;

		CheckForGround(groundHit);

		if (groundHit.IsValidBlockingHit())
		{
			FVector diff = groundHit.Location - groundTraceStartLocation;
			FVector stepCorrection = FVector::UpVector * -FMath::Abs(diff.Z);

			SafeMoveUpdatedComponent(stepCorrection, rotation, true, groundHit);
		}
	}

	{
		FHitResult groundHit;
		bIsOnGround = CheckForGround(groundHit);
	}

	/** Apply gravity */
	if (!bIsOnGround)
	{
		float desiredVerticalMovement = oldVelocity.Z + gravityZ * DeltaTime;
		float clampedVerticalMovement = FMath::Clamp(desiredVerticalMovement, -terminalVelocity, terminalVelocity);
		Velocity = desiredMovement + FVector::UpVector * clampedVerticalMovement;

		FVector verticalMovementDelta = FVector::UpVector * Velocity.Z * DeltaTime;

		SafeMoveUpdatedComponent(verticalMovementDelta, rotation, true, hit);

		if (hit.IsValidBlockingHit())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, hit.IsValidBlockingHit() ? FColor::Red : FColor::Green,
				FString::Printf(TEXT("Hit2=%s (N=%s)"), hit.Actor.IsValid() ? *hit.Actor->GetName() : TEXT("None"),
					*hit.Normal.ToCompactString()));
			Velocity.Z = 0;
		}
	}
	else
	{
		Velocity = desiredMovement;
	}

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Velocity=%s"), *Velocity.ToCompactString()));
	GEngine->AddOnScreenDebugMessage(
		-1, DeltaTime, bIsOnGround ? FColor::Green : FColor::Yellow, FString::Printf(TEXT("bIsOnGround")));
}

bool UMainCharacterMovementComponent::CheckForGround(FHitResult& OutHit) const
{
	if (UpdatedCollider)
	{
		/** Start slightly above location to avoid false-positives from initial penetrations */
		const static float startZOffset = 5.0f;
		const float capsuleRadius = UpdatedCollider->GetScaledCapsuleRadius();
		const float capsuleHalfHeight = UpdatedCollider->GetScaledCapsuleHalfHeight();

		const FVector groundTraceStartLocation = UpdatedCollider->GetComponentLocation() + FVector::UpVector * startZOffset;
		const FVector groundTraceEndLocation = groundTraceStartLocation + FVector::UpVector * (-MaxStepHeight - startZOffset);

		FCollisionQueryParams groundSweepParams;
		groundSweepParams.AddIgnoredActor(UpdatedCollider->GetOwner());

		/** Construct a flat box contained withing the capsule */
		const FCollisionShape groundSweepShape =
			FCollisionShape::MakeBox(FVector(capsuleRadius * UE_INV_SQRT_2, capsuleRadius * UE_INV_SQRT_2, capsuleHalfHeight));

		const FQuat rotated45deg = FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f);

		bool bHit = GetWorld()->SweepSingleByProfile(OutHit, groundTraceStartLocation, groundTraceEndLocation, rotated45deg,
			TEXT("BlockAll"), groundSweepShape, groundSweepParams);

		if (!bHit)
		{
			bHit = GetWorld()->SweepSingleByProfile(OutHit, groundTraceStartLocation, groundTraceEndLocation, FQuat::Identity,
				TEXT("BlockAll"), groundSweepShape, groundSweepParams);
		}

		return OutHit.IsValidBlockingHit();
	}

	return false;
}
