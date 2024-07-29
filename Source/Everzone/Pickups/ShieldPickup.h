// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AShieldPickup : public APickup
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
	UPROPERTY(EditAnywhere, Category = "Shield Pickup")
	float ShieldRegenAmount = 100.f;
	UPROPERTY(EditAnywhere, Category = "Shield Pickup")
	float RegenOverTime = 4.f;
};