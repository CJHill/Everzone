// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * This class is responsible for applying damage to the player
 */
UCLASS()
class EVERZONE_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
public:
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
private:

};
