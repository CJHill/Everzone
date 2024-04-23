// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EverzonePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzonePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
protected:
	virtual void BeginPlay() override;
private:

	UPROPERTY()
	class AEverzoneHUD* EverzoneHUD;
};
