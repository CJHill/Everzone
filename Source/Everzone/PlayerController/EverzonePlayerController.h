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
protected:
	virtual void BeginPlay() override;
private:
	class AEverzoneHUD* EverzoneHUD;
};
