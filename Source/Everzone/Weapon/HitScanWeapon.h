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
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutputHit);

    UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

    UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

   

private:
	

	

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
	class USoundCue* ShootSound;
	



	
};
