// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	virtual void Shoot(const FVector& HitTarget) override;
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumOfShotgunPellets = 10;
};
