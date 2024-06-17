// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeleeKnife.generated.h"

UCLASS()
class EVERZONE_API AMeleeKnife : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeleeKnife();
	UFUNCTION(Server, Reliable)
	void MulticastOnHit(AEverzoneCharacter* HitPlayer);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void KnifeDamage(AActor* OtherActor);
	void MeleeTraceHit();
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	class UParticleSystem* PlayerImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
private:
	UPROPERTY(VisibleAnywhere, Category = "Melee Properties")
	USkeletalMeshComponent* KnifeMesh;
	FHitResult MeleeHit;
	float Damage = 100;
public:	
	FORCEINLINE USkeletalMeshComponent* GetKnifeMesh() const { return KnifeMesh; }

};
