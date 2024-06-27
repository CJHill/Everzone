// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

//This component class is for handling functionality for the power up pickups ie heal buff, shield buff, jump buff.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EVERZONE_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UBuffComponent();
	friend class AEverzoneCharacter;
	void Heal(float HealAmount, float HealOverTime);
	void BuffSpeed(float BaseSpdIncrease, float BaseCrouchSpdIncrease, float SpdBuffTime);

	// SetInitialSpd will be called in post initialise components function in everzone character's cpp file passing in max walk speed and max walk speed crouched
	void SetInitialSpd(float BaseSpd, float BaseCrouchSpd); 
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	
	virtual void BeginPlay() override;
	void HealOverTime(float DeltaTime);
private:
	UPROPERTY()
	class AEverzoneCharacter* Character;

	//Speed buff properties
	FTimerHandle SpeedTimer;
	void ResetSpeedTimer();
	float InitialBaseSpd;
	float InitialCrouchSpd;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpdBuff(float BaseSpd, float BaseCrouchSpd);

	//Healing buff properties
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;


public:	
	
	

		
};
