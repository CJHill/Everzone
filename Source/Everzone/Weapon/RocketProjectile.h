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
	virtual void Destroyed() override;
protected:
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	void DestroyTimerFinished();

	UPROPERTY(VisibleAnywhere)
	class URocketProjectileMovementComp* RocketMovementComp;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	class UNiagaraSystem* NS_SmokeTrail;
	UPROPERTY()
	class UNiagaraComponent* NSComp_SmokeTrail;

	//Rocket Loop Sound Properties
	UPROPERTY(EditAnywhere)
	class USoundCue* RocketLoop;
	UPROPERTY()
	class UAudioComponent* RocketLoopComp;
	UPROPERTY(EditAnywhere)
	class USoundAttenuation* RocketLoop_Att;
private:
	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float InnerDamageRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float OuterDamageRadius = 450.f;
	UPROPERTY(VisibleAnywhere, Category = "Static Mesh")
	class UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere, Category = "Niagara")
		float DestroyTime = 3.f;
};
