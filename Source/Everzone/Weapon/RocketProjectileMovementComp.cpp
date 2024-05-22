// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectileMovementComp.h"

URocketProjectileMovementComp::EHandleBlockingHitResult URocketProjectileMovementComp::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketProjectileMovementComp::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop, they should only explode when their collision box detects a hit
}
