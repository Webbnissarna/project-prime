// Copyright 2021 Master Kenth

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"

#include "MainCharacterMovementComponent.generated.h"

/**
 *
 */
UCLASS()
class PRIME_API UMainCharacterMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	UPROPERTY()
	class UCapsuleComponent* UpdatedCollider;

	UMainCharacterMovementComponent();
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/** Check if we're on ground.
	 * @param OutHit ground hit result if we're on ground.
	 * @return true if we're on ground, false otherwise.
	 */
	bool CheckForGround(FHitResult& OutHit) const;
};
