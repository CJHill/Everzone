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
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void ShowDeathMessage(const FString KilledBy);
	void HideDeathMessage();
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDAmmoReserves(int32 Ammo);
	void ShowWeaponIcon(UTexture2D* WeaponIcon);
	void HideWeaponIcon();
	void SetHUDMatchTimer(float TimeRemaining);
	void SetHUDAnnouncementTimer(float TimeRemaining);
	// Synced server world clock function
	virtual float GetCurrentServerTime();
	//Syncs with the server clock as soon as possible
	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);
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
	UFUNCTION(Client, Reliable)
	void ReportServerTime(float TimeOfServerRequest, float TimeReceivedServerRequest); // Reports the server time in response to request server time function. Passes in the time when the request was made and the time the server received the request
	
	float ClientServerDeltaTime = 0.f; //difference between server and client time

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 2.f;

	float TimeSinceLastSync = 0.f;
	void RefreshTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void CheckMatchState(); // Server RPC for checking the match state as soon as the controller is set up

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfTheMatch, float Warmup,float Cooldown, float Match, float StartingTime);
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
	bool bInitCharacterOverlay = false;

	float HUDCurrentHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDeaths;

	
};
