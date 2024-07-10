// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class EVERZONE_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APickup();
    virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UPROPERTY(EditAnywhere, Category = "Pickups")
	float BaseTurnRate = 45.f;
private:
	UPROPERTY(EditAnywhere, Category = "Pickups")
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, Category = "Pickups")
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pickups")
		class UNiagaraComponent* PickupEffectComp;

	UPROPERTY(EditAnywhere, Category = "Pickups")
		class UNiagaraSystem* PickupEffect;

	// Properties for delaying overlap events. This is needed to ensure that the pickup gets spawned even if the player is sitting on the spawn point before the pickup is spawned in
	// Without these properties the pickup will be destroyed so quickly that no other functionality can be fired
	FTimerHandle OverlapTimer;
	float OverlapTimeDelay = 0.25f;
	void OverlapTimerFinished();
public:	

	

};
