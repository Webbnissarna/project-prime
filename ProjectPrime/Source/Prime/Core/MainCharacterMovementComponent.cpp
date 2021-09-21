// Copyright 2021 Master Kenth

#include "MainCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PhysicsVolume.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UMainCharacterMovementComponent::UMainCharacterMovementComponent()
	: WalkForce(1000.0f)
	, MaxSpeed(1200.0f)
	, UpdatedCollider(nullptr)
	, MaxStepHeight(20.0f)
	, MaxGroundAngle(60.0f)
	, bIsOnGround(false)
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

	FHitResult fricHit;
	FCollisionQueryParams qParams;
	qParams.AddIgnoredActor(GetOwner());
	qParams.bReturnPhysicalMaterial = true;
	GetWorld()->LineTraceSingleByChannel(fricHit, GetActorFeetLocation() + FVector::UpVector * 10.0f,
		GetActorFeetLocation() + FVector::UpVector * -10.0f, ECollisionChannel::ECC_Pawn, qParams);

	float mass = 95.0f;
	float friction = 0.7f;
	float frictionRatio = 0.1f;
	float leastForce = WalkForce * frictionRatio;
	float surfaceForce = leastForce + ((WalkForce - leastForce) * friction);
	FVector finalForce = surfaceForce * input;
	FVector acceleration = finalForce / mass;
	FVector idealVelocity = oldVelocity + acceleration * DeltaTime;

	FVector clampedNewVelocity = idealVelocity.GetClampedToMaxSize2D(MaxSpeed);
	Velocity = clampedNewVelocity;
	MoveUpdatedComponent(Velocity * DeltaTime, rotation, false);
	UpdateComponentVelocity();

	if (!input.IsNearlyZero())
		UE_LOG(LogTemp, Log, TEXT("|V|=%f, dt=%f"), Velocity.Size(), DeltaTime);
	return;

	/** Perform initial movement */
	FHitResult hit;
	SafeMoveUpdatedComponent(clampedNewVelocity, rotation, true, hit);

	FVector colliderLocation = UpdatedCollider->GetComponentLocation();
	float feetZ = colliderLocation.Z - UpdatedCollider->GetScaledCapsuleHalfHeight();
	FVector feetLocation = FVector(colliderLocation.X, colliderLocation.Y, feetZ);

	if (hit.IsValidBlockingHit())
	{
		FVector hitNormal = hit.Normal;
		float surfaceDot = hitNormal | FVector::UpVector;
		float surfaceAngleRad = FMath::Acos(surfaceDot);
		float surfaceAngle = FMath::RadiansToDegrees(surfaceAngleRad);

		float heightDiff = hit.ImpactPoint.Z - feetZ;

		bool bStepIsTallerThanMaxHeight = heightDiff > (MaxStepHeight + 0.5f);

		if (bStepIsTallerThanMaxHeight)
		{
			hitNormal.Z = 0;
			hitNormal.Normalize();
		}

		HandleImpact(hit, DeltaTime, clampedNewVelocity);

		SlideAlongSurface(clampedNewVelocity, 1.0f - hit.Time, hitNormal, hit, true);
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

		if (bIsOnGround)
		{
			const auto hitPhysMaterial = groundHit.PhysMaterial.Get();
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan,
				FString::Printf(TEXT("PhysicsMaterial: %s friction: %f (%f)"),
					hitPhysMaterial ? *hitPhysMaterial->GetName() : TEXT("unknown"),
					hitPhysMaterial ? hitPhysMaterial->Friction : -1.0f,
					hitPhysMaterial ? hitPhysMaterial->StaticFriction : -1.0f));
		}
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

		const FVector startLocation = UpdatedCollider->GetComponentLocation() + FVector::UpVector * startZOffset;
		const FVector endLocation = startLocation + FVector::UpVector * (-MaxStepHeight - startZOffset);

		FCollisionQueryParams sweepParams;
		sweepParams.AddIgnoredActor(UpdatedCollider->GetOwner());

		FCollisionResponseParams responseParams;
		UpdatedCollider->InitSweepCollisionParams(sweepParams, responseParams);

		/** Construct a flat box contained withing the capsule */
		const FCollisionShape sweepShape =
			FCollisionShape::MakeBox(FVector(capsuleRadius * UE_INV_SQRT_2, capsuleRadius * UE_INV_SQRT_2, capsuleHalfHeight));

		const FQuat rotated45deg = FQuat(FVector(0.f, 0.f, -1.f), PI * 0.25f);
		const ECollisionChannel collisionChannel = UpdatedComponent->GetCollisionObjectType();

		TArray<FHitResult> sweepHits;
		bool bHit = GetWorld()->SweepMultiByChannel(
			sweepHits, startLocation, endLocation, rotated45deg, collisionChannel, sweepShape, sweepParams, responseParams);

		bool bAnyValidGround = false;

		if (bHit)
		{
			bAnyValidGround = IsAnyHitValidGround(sweepHits, OutHit);
		}

		if (!bAnyValidGround)
		{
			bHit = GetWorld()->SweepMultiByChannel(
				sweepHits, startLocation, endLocation, rotated45deg, collisionChannel, sweepShape, sweepParams, responseParams);

			if (bHit)
			{
				bAnyValidGround = IsAnyHitValidGround(sweepHits, OutHit);
			}
		}

		return bAnyValidGround;
	}

	return false;
}

bool UMainCharacterMovementComponent::IsAnyHitValidGround(const TArray<FHitResult>& Hits, FHitResult& OutFirstValidHit) const
{
	for (const auto& hit : Hits)
	{
		float dot = FVector::UpVector | hit.ImpactNormal;
		float angle = FMath::Acos(dot);
		float angledeg = FMath::RadiansToDegrees(angle);
		if (angledeg <= MaxGroundAngle)
		{
			OutFirstValidHit = hit;
			return true;
		}
	}

	return false;
}