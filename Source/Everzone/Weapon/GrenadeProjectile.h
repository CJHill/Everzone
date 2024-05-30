// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "GrenadeProjectile.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AGrenadeProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	AGrenadeProjectile();
	virtual void Destroyed() override;
protected:

	virtual void BeginPlay() override;
	/*OnBounce:
	* Projectile movement component has an On Bounce Delegate that has a FHitResult and FVector parameter.
	* OnBounce will serve as a callback to this delegate and allow us to change the forces of the grenade's bounce
	*/
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	
private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
