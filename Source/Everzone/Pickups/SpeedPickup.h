// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere, Category = "Speed Pickups")
	float BaseSpdBuff = 1600.f;
	UPROPERTY(EditAnywhere, Category = "Speed Pickups")
	float BaseCrouchSpdBuff = 850.f;
	UPROPERTY(EditAnywhere, Category = "Speed Pickups")
	float SpdBuffTime = 10.f;
};
