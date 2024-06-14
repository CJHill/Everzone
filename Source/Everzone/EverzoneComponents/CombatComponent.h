// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Everzone/HUD/EverzoneHUD.h"
#include "Everzone/Weapon/WeaponTypes.h"
#include "Everzone/EverzoneTypes/CombatState.h"
#include "CombatComponent.generated.h"

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

	//Blueprint Callable functions that will be used in the Anim BP for notify events in the reload montages
	UFUNCTION(BlueprintCallable)
	void FinishedReload();
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	UFUNCTION(BlueprintCallable)
	void MeleeFinished();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	bool IsShotgun();
	void JumpToShotgunEnd();
    void ShootButtonPressed(bool bIsPressed);

	
protected:
	
	virtual void BeginPlay() override;
	//Refactor for the Equip Weapon Function. All these functions may be used elsewhere, when appropriate
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);

	void PlayEquipWeaponSound();
	void UpdateAmmoReserves();
	void ReloadEmptyWeapon();

	void AttachActorToLeftHand(AActor* ActorToAttach);

	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Shoot();

	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	void ShowAttachedGrenade(bool bShowGrenade);
	UPROPERTY(EditAnywhere, Category = "Grenades")
	TSubclassOf<class AProjectile> GrenadeClass;

	UFUNCTION(Server, Reliable)
	void ServerShoot(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShoot(const FVector_NetQuantize& TraceHitTarget);

	void Melee();
	UFUNCTION(Server, Reliable)
	void ServerMelee();
	UPROPERTY(EditAnywhere, Category = "Melee")
	TSubclassOf<class AMeleeKnife> MeleeClass;
	void ShowAttachedKnife(bool bShowKnife);
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

	// Starting ammo for weapons
	UPROPERTY(EditAnywhere)
	int32 InitialAmmoReserves = 30;
	UPROPERTY(EditAnywhere)
	int32 InitialRocketAmmoReserves = 5;
	UPROPERTY(EditAnywhere)
	int32 InitialPistolAmmoReserves = 15;
	UPROPERTY(EditAnywhere)
	int32 InitialSMGAmmoReserves = 25;
	UPROPERTY(EditAnywhere)
	int32 InitialShotgunAmmoReserves = 10;
	UPROPERTY(EditAnywhere)
	int32 InitialSniperAmmoReserves = 6;
	UPROPERTY(EditAnywhere)
	int32 InitialGrenadeLauncherAmmoReserves = 8;

	// Initialising Ammo Reserves
	void InitAmmoReserves();
	void UpdateAmmoAmount();
	void UpdateShotgunAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 2;
	UPROPERTY(EditAnywhere, Category = "Grenades")
	int32 MaxGrenades = 9;
	UFUNCTION()
	void OnRep_Grenades();
	void UpdateHUDGrenades();

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY()
	class UTexture2D* WeaponIconImage;
	void SetWeaponIcon();
public:	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	

		
};
