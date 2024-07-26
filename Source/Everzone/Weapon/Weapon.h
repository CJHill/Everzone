// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquipSecondary UMETA(DisplayName = "Secondary Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};
UCLASS()
class EVERZONE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);
	void SetHUDAmmo();
	virtual void OnRep_Owner() override;
    void Dropped();
	void AddAmmo(int32 AmmoToAdd);
/*
* Shoot(): virtual function that can be overidden as different weapon types that will be derived from this class may require unique functionality
*/
	virtual void Shoot(const FVector& HitTarget);

	/*
	* Textures for Crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairUp;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairDown;

	/*
	* Field of View variables for aiming can be changed in blueprints as different weapon types may need different zoom in speeds
	*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 35.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpd = 25.f; 

	/*
	* Automatic fire properties
	*/
	UPROPERTY(EditAnywhere, Category = "Combat")
		float ShootDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Combat")
		bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	class USoundCue* EquipSound;

	/*
	* Properties for enabling and disabling custom depth stencil. This is for giving the weapon mesh a glow outline effect
	*/
	void EnableCustomDepth(bool bEnable);


	bool bDestroyWeapon = false;
protected:
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);
	virtual void OnWeaponStateSet();
	virtual void HandleOnEquipped();
	virtual void HandleOnDropped();
	virtual void HandleOnEquipSecondary();
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
    EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* ShootAnim;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* WeaponIconTexture;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletShell> BulletShellClass;
	/*
	* Ammunition Properties
	*/
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;
	UPROPERTY(EditAnywhere)
	int32 AmmoMagazine;
	UFUNCTION()
	void OnRep_Ammo();
	void UseAmmo();

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	class AEverzoneCharacter* EverzoneOwningCharacter;
	class AEverzonePlayerController* EverzoneOwningController;
public:	
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	bool AmmoIsEmpty();
	bool AmmoIsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetAmmoMag() const { return AmmoMagazine; }
	FORCEINLINE UTexture2D* GetWeaponIcon() const { return WeaponIconTexture; }
};
