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

	void TraceCrosshairs(FHitResult& TraceHitResult);
private:
	AEverzoneCharacter* Character;

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
	
public:	
	
	

		
};
