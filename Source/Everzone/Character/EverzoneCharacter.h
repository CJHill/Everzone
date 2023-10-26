// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Everzone/EverzoneTypes/TurningInPlace.h"
#include "Everzone/Interfaces/CrosshairInterface.h"
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
	void PlayShootMontage(bool bAiming);
	
	virtual void OnRep_ReplicatedMovement() override;
	
protected:
	virtual void BeginPlay() override;
	void UpdateHUDHealth();
	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void ShootButtonPressed();
	void ShootButtonReleased();
	void PlayHitReactMontage();
	//AIMOFFSET():checks if equipped weapon is a nullptr and will return if true
	//Gets velocity and stores in a local float checks to see if speed is equal to 0 and player is not falling
	//
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	float CalculateSpeed();
	void SimProxyRotate();
	virtual void Jump() override;
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* ShootMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* HitReactMontage;

	//Using a rep notify function because they don't get called on the server meaning that setting the pickup widget's visibility inside OnRep_OverlappingWeapon
	// will allow the display text to only appear on the client that owns the pawn overlapping with the actor
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComp;

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
	void OnRep_Health();

	class AEverzonePlayerController* PlayerController;
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	//Getter function that returns true if the weapon is equipped
	bool IsWeaponEquipped();
	//Getter function that returns true if the player is aiming
	bool IsAiming();
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetAO_Yaw() const  { return AO_Yaw; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool RotateRootBone() const { return bRotateRootBone; }
};
