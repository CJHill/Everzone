// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class EVERZONE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	/*
	* The Destroyed Function is an override from the parent function in Unreal Engine's AActor.h class, In the parent function of Destroyed(), it says this function will be called
	* whenever the actor is destroyed during gameplay or in the editor. This parent function of Destroyed() will also broadcast itself using a multicast delegate, which means
	* that once the actor has been destroyed that information will be replicated on the server and to all remaining clients.
	*/
	virtual void Destroyed() override;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnHit(AEverzoneCharacter* HitPlayer);
protected:
	
	virtual void BeginPlay() override;
	   
		//HitFunction that will be bound to the OnComponentHit Multicast Delegate found in PrimitiveComponent.h the parameters are the same in order for the binding to work
		UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
		UPROPERTY(EditAnywhere)
		float Damage = 15.f;
private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComp;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComp;

	//Impact Variables are needed for the OnHit function

	UPROPERTY(EditAnywhere)
	UParticleSystem* DefaultParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles; 

	UPROPERTY(EditAnywhere)
	UParticleSystem* PlayerImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	
public:	
	
	

};
