// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Everzone/HUD/EverzoneHUD.h"
#include "Everzone/Weapon/WeaponTypes.h"
#include "Everzone/EverzoneTypes/CombatState.h"
#include "CombatComponent.generated.h"
#define TRACE_LENGTH 80000.f
class AWeapon;// forward delcaring as this class will be important for combat
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EVERZONE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AEverzoneCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishedReload();
    void ShootButtonPressed(bool bIsPressed);
protected:
	
	virtual void BeginPlay() override;

	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Shoot();

	

	UFUNCTION(Server, Reliable)
	void ServerShoot(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShoot(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();
	/*
	* TraceCrosshairs(): calculates the position of the crosshair at the center of the game viewport,
	* converts it from screen coordinates to world coordinates, and performs a line trace from that position to detect collisions with the game world. 
	* Depending on whether a collision is detected or not, it updates the TraceHitResult accordingly;
	*/
	void TraceCrosshairs(FHitResult& TraceHitResult);

	/*
	* SetHUDCrosshair(): Performs an conditional check to see if the Everzone character or controller inherited from Unreal code is nullptr 
	* Checks to see if our player controller and playerHUD is equal to nullptr if true we use a cast to get the controller and HUD that's found in unreal engine code
	* and store this inside our own player controller and hud variable, in the event its false we simply store in our player controller and HUD.
	* If we have an equipped weapon we store our crosshair textures found on the weapon class and inside our blueprint into the FHUDPackage's crosshair texture variables
	* if we dont have a weapon we assign the FHUDPackage's crosshairs to nullptr. 
	* We use our crosshair factor variables to calculate how big or small the crosshair should be based on the player's current state i.e crouched , aiming, shooting 
	* At the end we call the SetHUDPackage passing in the FHUDPackage variable here on Combat Component
	*/
	void SetHUDCrosshair(float DeltaTime);
private:
	UPROPERTY()
	AEverzoneCharacter* Character;
	UPROPERTY()
	class AEverzonePlayerController* PlayerController;
	UPROPERTY()
	class AEverzoneHUD* PlayerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(Replicated)
	bool bShootIsPressed;

	/*
	* HUD and Crosshairs
	*/
	FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootFactor;
	float AimInterpTarget = -0.58f;
	float CharacterTraceFactor;
	FVector HitTarget;
	bool TraceAgainstCharacter;

	/*
	* Aiming and Field of view properties
	*/

	//Fov when not aiming
	float DefaultFov;

	float CurrentFov;

	UPROPERTY(EditAnywhere)
	float ZoomedFov = 35.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 25.f;

	void InterpFov(float DeltaTime);
	/*
	* Automatic Fire properties
	*/
	FTimerHandle ShootTimer;
	bool bCanShoot = true;

	void StartShootTimer();
	void EndShootTimer();
	bool CanShoot();

	//Ammo reserves for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_AmmoReserves)
	int32 AmmoReserves;

	UFUNCTION()
	void OnRep_AmmoReserves();

	TMap<EWeaponType, int32> AmmoReservesMap;

	UPROPERTY(EditAnywhere)
	int32 InitialAmmoReserves = 30;

	// Initialising Ammo Reserves
	void InitAmmoReserves();
	void UpdateAmmoAmount();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY()
	class UTexture2D* WeaponIconImage;
	void SetWeaponIcon();
public:	
	
	

		
};
