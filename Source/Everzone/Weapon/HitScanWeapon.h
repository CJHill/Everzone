// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Shoot(const FVector& HitTarget) override;
protected:
	FVector TraceEndPointWithScatter(const FVector& TraceStart, const FVector& TraceEnd);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutputHit);

    UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere)
	float Damage = 10.f;

private:
	

	

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
	class USoundCue* ShootSound;
	

	/*
	* Calculating end point for line trace with scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 80.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
};
