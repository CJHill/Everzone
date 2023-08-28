// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EverzoneCharacter.generated.h"

UCLASS()
class EVERZONE_API AEverzoneCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEverzoneCharacter();
	virtual void Tick(float DeltaTime) override;
    // Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
protected:
	virtual void BeginPlay() override;
	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	void EquipButtonPressed();
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

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
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	//Getter function that returns true if the weapon is equipped
	bool IsWeaponEquipped();
};
