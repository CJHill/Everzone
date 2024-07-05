// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Everzone/EverzoneTypes/TurningInPlace.h"
#include "Everzone/Interfaces/CrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "Everzone/EverzoneTypes/CombatState.h"
#include "EverzoneCharacter.generated.h"

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
	virtual void OnRep_ReplicatedMovement() override;
	//We have two eliminated functions multicasteliminated handles functionality being replicated to all clients. Eliminated just handles server functionality
	void Eliminated();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();
	virtual void Destroyed() override;

	UFUNCTION(BlueprintImplementableEvent) // Showing the sniper scope will be handled by our blueprint class
	void ShowSniperScope(bool bShowScope);
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	void UpdateHUDHealth();
	void UpdateHUDShield();
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

private:
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

	//Using a rep notify function because they don't get called on the server meaning that setting the pickup widget's visibility inside OnRep_OverlappingWeapon
	//will allow the display text to only appear on the client that owns the pawn overlapping with the actor
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComp;
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComp;

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
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMatInst;

	UFUNCTION()
	void UpdateDissolveMat(float DissolveMatValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

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
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComp() const { return CombatComp; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComp() const { return BuffComp; }
};
