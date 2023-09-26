// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
protected:
	
	virtual void BeginPlay() override;

	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void ShootButtonPressed(bool bIsPressed);

	UFUNCTION(Server, Reliable)
	void ServerShoot(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShoot(const FVector_NetQuantize& TraceHitTarget);
	/*
	* TraceCrosshairs(): calculates the position of the crosshair at the center of the game viewport,
	* converts it from screen coordinates to world coordinates, and performs a line trace from that position to detect collisions with the game world. 
	* Depending on whether a collision is detected or not, it updates the TraceHitResult accordingly;
	*/
	void TraceCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshair(float DeltaTime);
private:
	AEverzoneCharacter* Character;
	class AEverzonePlayerController* PlayerController;
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
	float CrosshairVelocityFactor;
	float CrosshairInAir;

	FVector HitTarget;
	
public:	
	
	

		
};
