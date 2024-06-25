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
virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	
	virtual void BeginPlay() override;
	void HealOverTime(float DeltaTime);
private:
	UPROPERTY()
	class AEverzoneCharacter* Character;

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
public:	
	
	

		
};
