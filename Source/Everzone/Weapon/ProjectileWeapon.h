// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Shoot(const FVector& HitTarget) override;
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
