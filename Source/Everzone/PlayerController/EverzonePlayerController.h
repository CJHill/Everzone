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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDShield(float ShieldAmount, float MaxShieldAmount);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void ShowDeathMessage(const FString KilledBy);
	void HideDeathMessage();
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDAmmoReserves(int32 Ammo);
	void SetHUDGrenades(int32 Grenades);
	void ShowWeaponIcon(UTexture2D* WeaponIcon);
	void HideWeaponIcon();
	void SetHUDMatchTimer(float TimeRemaining);
	void SetHUDAnnouncementTimer(float TimeRemaining);
	// Synced server world clock function
	virtual float GetCurrentServerTime();
	//Syncs with the server clock as soon as possible
	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	float SingleTripTime = 0.f;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	void HandleMatchHasStarted();
	/*
	* Handle Cooldown: enables the announcment overlay checks the Game State's player array for top scorers, display's the winners name and disables gameplay ie shoot aim interact
	*/ 
	void HandleCooldown();
	/*
	* Sync time between client and server properties
	*/
	UFUNCTION(Server, Reliable)
	void RequestServerTime(float TimeServerRequest); //Requests the current time on the server. Passes in the time when the request was made
	//ReportServerTime: Reports the server time in response to request server time function. Passes in the time when the request was made and the time the server received the request
	UFUNCTION(Client, Reliable)
	void ReportServerTime(float TimeOfServerRequest, float TimeReceivedServerRequest); 
	
	float ClientServerDeltaTime = 0.f; //difference between server and client time

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 2.f;

	float TimeSinceLastSync = 0.f;
	void RefreshTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void CheckMatchState(); // Server RPC for checking the match state as soon as the controller is set up

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfTheMatch, float Warmup,float Cooldown, float Match, float StartingTime);

	void ShowHighPingWarning();
	void HideHighPingWarning();
	void CheckPing(float DeltaTime);
private:
	UPROPERTY()
	class AEverzoneHUD* EverzoneHUD;
	UPROPERTY()
	class AEverzoneGameMode* EverzoneGameMode;
	UPROPERTY()
	class AEverzoneCharacter* EverzoneCharacter;
	float LevelStartTime = 0.f;
	
	float MatchTime = 0.f;
	
	float WarmUpTime = 0.f;

	float CooldownTime = 0.f;
	
	uint32 MatchTimerInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName StateofMatch;
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UOverlayWidget* CharacterOverlay;
	bool bInitHUDHealth = false;
	bool bInitHUDShield = false;
	bool bInitHUDScore = false;
	bool bInitHUDDeath = false;
	bool bInitHUDGrenade = false;
	bool bInitHUDAmmoReserves = false;
	bool bInitHUDWeaponAmmo = false;
	bool bInitHUDWeaponIcon = false;
	float HUDCurrentHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDeaths;
	int32 HUDGrenades;
	int32 HUDWeaponAmmo;
	int32 HUDAmmoReserves;
	UPROPERTY()
	UTexture2D* HUDWeaponIcon;


	//Ping Variables needed for displaying the ping issue symbol at appropriate times
	float HighPingTimeShown = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	float HighPingThreshold = 50.f;

	float PingAnimationDisplayTime = 0.f;
	float CurrentPing = 0.f;
};
