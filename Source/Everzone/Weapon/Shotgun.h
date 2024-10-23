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
	/*
	* The unique shoot functionality for the shotgun creates a TMap to contain the number of hits. 
	  Uses a for loop the shotgun pellets so each pellet is conducting a line trace. A hit is recorded getting a pointer of the character that was hit and storing it in the TMap with a value of 1 per hit.
	  Damage is then applied after this code is conducted and it is multiplied by the value of the Key pair.
	*/
	virtual void ShootShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndPointWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>&HitTargets);
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumOfShotgunPellets = 10;
};
