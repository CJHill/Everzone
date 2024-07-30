// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Everzone/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
public:
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;
private:
	UPROPERTY(EditAnywhere, Category = "Ammo Pickups")
	int32 AmountOfAmmo = 30;
	UPROPERTY(EditAnywhere, Category = "Ammo Pickups")
	EWeaponType WeaponType;
};
