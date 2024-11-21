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

	/*Used For Server Side Rewind*/
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
protected:
	
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
		//HitFunction that will be bound to the OnComponentHit Multicast Delegate found in PrimitiveComponent.h the parameters are the same in order for the binding to work
		UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
		UPROPERTY(EditAnywhere)
		float Damage = 15.f;
		void ExplosionDamage();
		UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;
		//Impact Variables are needed for the OnHit function

		UPROPERTY(EditAnywhere)
		class UParticleSystem* DefaultParticles;
		UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;
		UPROPERTY(EditAnywhere)
		class UParticleSystem* PlayerImpactParticles;
		UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

        UPROPERTY(VisibleAnywhere)
	    class UProjectileMovementComponent* ProjectileMovementComp;

		UPROPERTY(EditAnywhere, Category = "Niagara")
		class UNiagaraSystem* NS_SmokeTrail;
		UPROPERTY()
		class UNiagaraComponent* NSComp_SmokeTrail;
		void SpawnProjectileTrail();

		UPROPERTY(VisibleAnywhere, Category = "Static Mesh")
		class UStaticMeshComponent* ProjectileMesh;
private:
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComp;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere, Category = "Niagara")
	float DestroyTime = 3.f;
	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float InnerDamageRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Damage Radii")
	float OuterDamageRadius = 450.f;
public:	
	
	

};
