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

	//Exact same purpose as found in Projectile bullet class see that for more info
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif
protected:
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	

	UPROPERTY(VisibleAnywhere)
	class URocketProjectileMovementComp* RocketMovementComp;

	

	//Rocket Loop Sound Properties
	UPROPERTY(EditAnywhere)
	class USoundCue* RocketLoop;
	UPROPERTY()
	class UAudioComponent* RocketLoopComp;
	UPROPERTY(EditAnywhere)
	class USoundAttenuation* RocketLoop_Att;
private:
	
	

	
};
