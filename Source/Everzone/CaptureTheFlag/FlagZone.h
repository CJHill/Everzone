// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Everzone/EverzoneTypes/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class EVERZONE_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;
protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* ZoneSphere;
public:	
	

};
