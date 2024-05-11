// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"
#include "EverzonePlayerController.generated.h"


/**
 * Responsiblities for this class are to; implement functionality for changing elements on the Player's HUD, syncing the time between server and client
 */
UCLASS()
class EVERZONE_API AEverzonePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//OnPossess is needed for reseting the health bar to full on respawn
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime);
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void ShowDeathMessage(const FString KilledBy);
	void HideDeathMessage();
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDAmmoReserves(int32 Ammo);
	void ShowWeaponIcon(UTexture2D* WeaponIcon);
	void HideWeaponIcon();
	void SetHUDMatchTimer(float TimeRemaining);
	// Synced server world clock function
	virtual float GetCurrentServerTime();
	//Syncs with the server clock as soon as possible
	virtual void ReceivedPlayer() override;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	/*
	* Sync time between client and server properties
	*/
	UFUNCTION(Server, Reliable)
	void RequestServerTime(float TimeServerRequest); //Requests the current time on the server. Passes in the time when the request was made
	UFUNCTION(Client, Reliable)
	void ReportServerTime(float TimeOfServerRequest, float TimeReceivedServerRequest); // Reports the server time in response to request server time function. Passes in the time when the request was made and the time the server received the request
	
	float ClientServerDeltaTime = 0.f; //difference between server and client time

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSinceLastSync = 0.f;
	void RefreshTimeSync(float DeltaTime);
private:
	UPROPERTY()
	class AEverzoneHUD* EverzoneHUD;

	float MatchTime = 120.f;
	uint32 MatchTimerInt = 0;
};
