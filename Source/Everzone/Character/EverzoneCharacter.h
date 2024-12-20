// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Everzone/EverzoneTypes/TurningInPlace.h"
#include "Everzone/Interfaces/CrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "Everzone/EverzoneTypes/CombatState.h"
#include "Everzone/EverzoneComponents/LagCompensationComponent.h"
#include "Everzone/EverzoneTypes/Team.h"
#include "EverzoneCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class EVERZONE_API AEverzoneCharacter : public ACharacter, public ICrosshairInterface
{
	GENERATED_BODY()

public:
	AEverzoneCharacter();
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	//Animation Montage Functions
	void PlayShootMontage(bool bAiming);
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void PlayMeleeMontage();
	void PlaySwapWeaponMontage();

	virtual void OnRep_ReplicatedMovement() override;
	//We have two eliminated functions multicasteliminated handles functionality being replicated to all clients. Eliminated just handles server functionality
	void Eliminated(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated(bool bPlayerLeftGame);

	UFUNCTION(Server, Reliable)
	void ServerLeftGame();
	FOnLeftGame OnLeftGame;

	virtual void Destroyed() override;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	UFUNCTION(BlueprintImplementableEvent) // Showing the sniper scope will be handled by our blueprint class
	void ShowSniperScope(bool bShowScope);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false; // This is to disable shooting once the game ends

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();

	bool bFinishedSwapping = false;
	//TMap for hitboxes
	UPROPERTY()
	TMap<FName, class UBoxComponent*> PlayerHitBoxes;

	//function for setting the material colour on the character
	void SetTeamColours(ETeam Team);
protected:
	virtual void BeginPlay() override;
	
	//GetAndInitHUD: Get and initialise relevent properties for the players HUD
	void GetAndInitHUD();
	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void ShootButtonPressed();
	void ShootButtonReleased();
	void GrenadeButtonPressed();
	void MeleeButtonPressed();
	void PlayHitReactMontage();
	//AIMOFFSET():checks if equipped weapon is a nullptr and will return if true
	//Gets velocity and stores in a local float checks to see if speed is equal to 0 and player is not falling
	//
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	float CalculateSpeed();
	void RotateInPlace(float DeltaTime);
	void SimProxyRotate();
	virtual void Jump() override;
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void HandleWeaponsOnDeath();
private:
	UPROPERTY()
	class AEverzoneGameMode* EverzoneGameMode;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	// Anim Montages
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ShootMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapWeaponMontage;

	//Using a rep notify function because they don't get called on the server meaning that setting the pickup widget's visibility inside OnRep_OverlappingWeapon
	//will allow the display text to only appear on the client that owns the pawn overlapping with the actor
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	//Everzone Component Classes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComp;
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComp;
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComp;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);



	/*
	* HideCamera(): This function's purpose is to hide the camera when the camera is too close to the character this notably happens when the character is pressed up against a wall
	* It will be called in the Tick function but it's conditional check will ensure it only happens in such a scenario as previously mentioned
	*/
	void HideCamera();

	UPROPERTY(EditAnywhere)
	float CameraTransition = 200.f;

	/*
	* Properties needed for SimProxyRotateFunction which handles turning in place for simulated proxies
	*/
	bool bRotateRootBone;
	float SimYawThreshold = 0.5f;
	FRotator SimProxyRotationLastFrame;
	FRotator SimProxyRotation;
	float SimProxyYaw;
	float TimeSinceLastSimReplication;

	/*
	* Health properties
	*/
	UPROPERTY(EditAnywhere, Category = "Player Properties")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Properties")
	float CurrentHealth = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealthValue);

	/*
	* Shield Properties
	*/

	UPROPERTY(EditAnywhere, Category = "Player Properties")
	float MaxShield = 100;

	UPROPERTY(ReplicatedUsing = OnRep_Shield,VisibleAnywhere, Category = "Player Properties")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShieldValue);

	/*
	* Elimination properties
	*/
	bool bIsEliminated;
	FTimerHandle EliminatedTimer;

	UPROPERTY(EditDefaultsOnly)
	float EliminatedDelay = 3.f;

	void EliminatedTimerFinished();

	bool bLeftGame = false;
	

	
	//Crown effect properties
	UPROPERTY(EditAnywhere, Category = "VFX")
	class UNiagaraSystem* CrownSystem;
	UPROPERTY()
	class UNiagaraComponent* CrownSystemComp;

	/*
	* Dissolve effect properties for elimination
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	//InstDynamicDissolveMat is the material being applied to the character at runtime when player is eliminated
	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* InstDynamicDissolveMat;

	//DissolveMatInst is the default material that can be set in the editor
	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMatInst;

	UFUNCTION()
	void UpdateDissolveMat(float DissolveMatValue);
	void StartDissolve();
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/*
	* Team colours material properties
	*/
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* OrangeDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* OrangeMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* BlueMaterial;
	
	UPROPERTY(EditAnywhere)
	UMaterialInstance* DefaultMaterial;
	

	/*
	* DeathBot Properties which is tied to elimination
	*/
	UPROPERTY(EditAnywhere)
	UParticleSystem* DeathBotEffect;
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* DeathBotComp;
	UPROPERTY(EditAnywhere)
	class USoundCue* DeathBotCue;

	UPROPERTY()
	class AEverzonePlayerController* PlayerController;

	UPROPERTY()
	class AEverzonePlayerState* EverzonePlayerState;

	//Grenade
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	//Default weapon
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;


	/*Hit Boxes for Server Side Rewind. Naming convention is the same as the corresponding bodypart found in the skeletal mesh BP*/
	UPROPERTY(EditAnywhere)
	class UBoxComponent* Head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Upperbackpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_r;
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	//Getter function that returns true if the weapon is equipped
	bool IsWeaponEquipped();
	//Getter function that returns true if the player is aiming
	bool IsAiming();
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool RotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float SetHealth(float HealAmount) { return CurrentHealth = HealAmount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float SetShield(float RegenAmount) { return Shield = RegenAmount; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComp() const { return CombatComp; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComp() const { return BuffComp; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComp() const { return LagCompensationComp; }
};
