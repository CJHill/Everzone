// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"
#include "EverzonePlayerController.generated.h"


/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzonePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void ShowDeathMessage(const FString KilledBy);
	void HideDeathMessage();
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDAmmoReserves(int32 Ammo);
	void ShowWeaponIcon(UTexture2D* WeaponIcon);
	void HideWeaponIcon();
protected:
	virtual void BeginPlay() override;
private:

	UPROPERTY()
	class AEverzoneHUD* EverzoneHUD;
};
