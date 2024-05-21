// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RocketProjectile.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API ARocketProjectile : public AProjectile
{
	GENERATED_BODY()
public:
	ARocketProjectile();
protected:
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float InnerDamageRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float OuterDamageRadius = 450.f;
	UPROPERTY(VisibleAnywhere, Category = "Static Mesh")
	class UStaticMeshComponent* RocketMesh;
};
