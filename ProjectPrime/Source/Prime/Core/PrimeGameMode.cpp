// Copyright 2021 Master Kenth

#include "PrimeGameMode.h"

void APrimeGameMode::BeginPlay()
{
  Super::BeginPlay();
  UE_LOG(LogTemp, Log, TEXT("Prime Game Mode!"));
}