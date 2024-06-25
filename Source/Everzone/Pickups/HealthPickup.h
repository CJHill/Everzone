// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
	virtual void Destroyed() override;
	
protected:
	
		virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere, Category = "Health Pickups")
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health Pickups")
	float HealOverTime = 4.f;

	

	UPROPERTY(VisibleAnywhere, Category = "Health Pickups")
	class UNiagaraComponent* PickupEffectComp;

	UPROPERTY(EditAnywhere, Category = "Health Pickups")
	class UNiagaraSystem* PickupEffect;

	
};
