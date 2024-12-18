// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Everzone/Weapon/Weapon.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "UObject/ConstructorHelpers.h"
#include "Everzone/Character/EverzoneAnimInstance.h"
#include "Everzone/Weapon/Projectile.h"
#include "Everzone/Weapon/MeleeKnife.h"
#include "Everzone/Weapon/Shotgun.h"

UCombatComponent::UCombatComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;
}





void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetFollowCamera())
		{
			DefaultFov = Character->GetFollowCamera()->FieldOfView;
			CurrentFov = DefaultFov;
		}
		if (Character->HasAuthority())
		{
			InitAmmoReserves();
		}

	}
	
}
void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
		
	}
}
void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, AmmoReserves, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCombatComponent, Grenades, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState)
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		
		FHitResult HitResult;
		TraceCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		InterpFov(DeltaTime);
	}
	
}

void UCombatComponent::SetAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bIsAiming = bAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		Character->ShowSniperScope(bAiming);
	}
	if(Character->IsLocallyControlled()) bAimButtonPressed = bAiming;
}
void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
void UCombatComponent::Shoot()
{
	if (CanShoot())
	{
		bCanShoot = false;
		
		if (EquippedWeapon)
		{
			CrosshairShootFactor = 0.9f;
			switch (EquippedWeapon->ShotType)
			{
			case ETypeOfShot::ETOS_Projectile:
				ShootProjectileWeapon();
				break;
			case ETypeOfShot::ETOS_HitScan:
				ShootHitScanWeapon();
				break;
			case ETypeOfShot::ETOS_Shotgun:
				ShootShotgun();
				break;
			}
		}
		StartShootTimer();
	}
	
}

void UCombatComponent::ShootProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndPointWithScatter(HitTarget) : HitTarget;
		
		if (!Character->HasAuthority()) LocalShoot(HitTarget);
		ServerShoot(HitTarget, EquippedWeapon->ShootDelay);
	}
}

void UCombatComponent::ShootHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndPointWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalShoot(HitTarget);
		ServerShoot(HitTarget, EquippedWeapon->ShootDelay);
	}
}

void UCombatComponent::ShootShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun)
	{
        TArray<FVector_NetQuantize>HitTargets;
		Shotgun->ShotgunTraceEndPointWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) LocalShootShotgun(HitTargets);
		ServerShootShotgun(HitTargets, EquippedWeapon->ShootDelay);
	}
	

}

void UCombatComponent::LocalShoot(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayShootMontage(bIsAiming);
		EquippedWeapon->Shoot(TraceHitTarget);
	}
}

void UCombatComponent::LocalShootShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bIsLocallyReloading = false;
		Character->PlayShootMontage(bIsAiming);
		Shotgun->ShootShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
		
	}

}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || !EquippedWeapon) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
       Character->PlayThrowGrenadeMontage();
	   AttachActorToLeftHand(EquippedWeapon);
	   ShowAttachedGrenade(true);
	}
	if (Character && !Character->HasAuthority())
	{
       ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}
void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
	
}
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
        ServerLaunchGrenade(HitTarget);
	}
	
}
void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	ShowAttachedGrenade(false);
	if (Character  && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector SpawnLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - SpawnLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (!World) return;
		World->SpawnActor<AProjectile>(GrenadeClass, SpawnLocation, ToTarget.Rotation(), SpawnParams);
		
	}
}
void UCombatComponent::UpdateHUDGrenades()
{
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDGrenades(Grenades);
	}
}
void UCombatComponent::ShootButtonPressed(bool bIsPressed)
{
	 bShootIsPressed = bIsPressed;
	 if (bShootIsPressed && EquippedWeapon)
	 {
		 Shoot();
	 }
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (AmmoReservesMap.Contains(WeaponType))
	{
		AmmoReservesMap[WeaponType] = FMath::Clamp(AmmoReservesMap[WeaponType] + AmmoAmount, 0, MaxAmmoReserves);
		UpdateAmmoReserves();
	}
	if (EquippedWeapon && EquippedWeapon->AmmoIsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		ReloadEmptyWeapon();
	}

}
void UCombatComponent::PickupGrenades(int32 GrenadesAmount)
{
	if (!GrenadesAmount) return;
	Grenades = FMath::Clamp(Grenades + GrenadesAmount, 0, MaxGrenades);
	UpdateHUDGrenades();

}
void UCombatComponent::SetSpeeds(float BaseSpd, float BaseCrouchSpd)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	AimWalkSpeed = BaseSpd;
	BaseWalkSpeed = BaseSpd;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpd;
}



void UCombatComponent::StartShootTimer()
{
	if (EquippedWeapon == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(ShootTimer, this, &UCombatComponent::EndShootTimer, EquippedWeapon->ShootDelay);
}

void UCombatComponent::EndShootTimer()
{ 
	if (EquippedWeapon == nullptr) return;
	bCanShoot = true;
	if (bShootIsPressed && EquippedWeapon->bIsAutomatic)
	{
		
		Shoot();
	}
	if (EquippedWeapon->AmmoIsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanShoot()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}
	
	if (!EquippedWeapon->AmmoIsEmpty() && bCanShoot && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		// if the player is using a shotgun, is reloading and has ammo in the shotgun we can interupt the reload process by shooting
		return true;
	}
	if (bIsLocallyReloading) return false;
	return !EquippedWeapon->AmmoIsEmpty() && bCanShoot && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_AmmoReserves()
{
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDAmmoReserves(AmmoReserves);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading && 
		EquippedWeapon != nullptr && 
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun
		&& AmmoReserves == 0;

	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitAmmoReserves() // Setting the Ammo Reserves for each weapon
{
	AmmoReservesMap.Emplace(EWeaponType::EWT_AssaultRifle, InitialAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_RocketLauncher, InitialRocketAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_Pistol, InitialPistolAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_SMG, InitialSMGAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_Shotgun, InitialShotgunAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_Sniper, InitialSniperAmmoReserves);
	AmmoReservesMap.Emplace(EWeaponType::EWT_GrenadeLauncher, InitialGrenadeLauncherAmmoReserves);
}

void UCombatComponent::ServerShoot_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastShoot(TraceHitTarget);
}
bool UCombatComponent::ServerShoot_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->ShootDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShoot_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShoot(TraceHitTarget);
}
void UCombatComponent::ServerShootShotgun_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShootShotgun(TraceHitTargets);
}
bool UCombatComponent::ServerShootShotgun_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->ShootDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}
void UCombatComponent::MulticastShootShotgun_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShootShotgun(TraceHitTargets);
}

/* Checks to see if the character and weapon to equip variable is equal to null if it is then it calls return.Otherwise it changes the weapon state to equipped
* Gets the hand socket the from the characters skeleton in the editor
* lastly gives ownership of the equipped weapon to the character in possession of the weapon and hides the pick up widget from display
 */
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
       EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
       EquipPrimaryWeapon(WeaponToEquip);
	}
	

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;
	Character->PlaySwapWeaponMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_SwappingWeapons;

	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}
void UCombatComponent::EquipPrimaryWeapon(AWeapon* PrimaryWeapon)
{
	if (!PrimaryWeapon) return;
	DropEquippedWeapon();
	EquippedWeapon = PrimaryWeapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateAmmoReserves();
	PlayEquipWeaponSound(PrimaryWeapon);
	ReloadEmptyWeapon();
	EquippedWeapon->EnableCustomDepth(false);
}
void UCombatComponent::EquipSecondaryWeapon(AWeapon* SecondWeapon)
{
	if (!SecondWeapon) return;
	SecondaryWeapon = SecondWeapon;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquipSecondary);
	SecondaryWeapon->SetOwner(Character);
	PlayEquipWeaponSound(SecondWeapon);
	AttachActorToBackpack(SecondWeapon);

}
void UCombatComponent::OnRep_EquippedWeapon()
{
	/*
	* We are setting the weapon state here to ensure that physics is disabled before attaching the weapon to the character
	* on all clients as only setting the weapon state on the server doesn't consider poor network performance may result in the attachment of the weapon taking place
	* before physics are disabled by the weapon state
	*/
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		EquippedWeapon->SetHUDAmmo();
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->ShowWeaponIcon(EquippedWeapon->GetWeaponIcon());
		}
	}
}
void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquipSecondary);
		AttachActorToBackpack(SecondaryWeapon);
		SecondaryWeapon->GetWeaponMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		SecondaryWeapon->GetWeaponMesh()->MarkRenderStateDirty();
	}
}
void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bShootIsPressed)
		{
			Shoot();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_Melee:
		if (Character && !Character->IsLocallyControlled())
		{
			AttachActorToLeftHand(EquippedWeapon);
			SpawnKnifeActor();
			Character->PlayMeleeMontage();
			
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapWeaponMontage();
		}
	}
}
void UCombatComponent::SetWeaponIcon()
{
	if (EquippedWeapon == nullptr) return;
	if (!WeaponIconImage) return;
	if (PlayerController == nullptr) return;
	WeaponIconImage = EquippedWeapon->GetWeaponIcon();
}
bool UCombatComponent::bShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}
void UCombatComponent::Reload()
{
	if (AmmoReserves > 0 && CombatState ==ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->AmmoIsFull() && !bIsLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bIsLocallyReloading = true;
	}
}
void UCombatComponent::FinishedReload()
{
	if (Character == nullptr) return;
	bIsLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoAmount();
	}
	if (bShootIsPressed)
	{
		Shoot();
	}
}
void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
       UpdateShotgunAmmo();
	}
	
}
bool UCombatComponent::IsShotgun()
{
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	else return false;
}
void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
}
void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon->AmmoIsEmpty())
	{
		Reload();
	}
}
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach || !EquippedWeapon) return;
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::Melee()
{
	if (CombatState != ECombatState::ECS_Unoccupied || !EquippedWeapon) return;
	CombatState = ECombatState::ECS_Melee;
	if (Character)
	{
		AttachActorToLeftHand(EquippedWeapon);
		SpawnKnifeActor();
		if (!KnifeActor) return;
		AttachActorToRightHand(Cast<AActor>(KnifeActor));
		Character->PlayMeleeMontage();
		
	}
	if (Character && !Character->HasAuthority())
	{
		ServerMelee();
	}
}
void UCombatComponent::ServerMelee_Implementation()
{
	CombatState = ECombatState::ECS_Melee;
	if (Character && MeleeClass)
	{
		AttachActorToLeftHand(EquippedWeapon);
		SpawnKnifeActor();
		if (!KnifeActor) return;
		AttachActorToRightHand(Cast<AActor>(KnifeActor));
		Character->PlayMeleeMontage();
		
	}
}

void UCombatComponent::SpawnKnifeActor()
{
	if (Character && Character->IsLocallyControlled())
	{
		ServerSpawnKnifeActor();
	}
	if (Character && MeleeClass)
	{
		const FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(FName("KnifeSocket"));
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (!World) return;
		KnifeActor = World->SpawnActor<AMeleeKnife>(MeleeClass, SpawnLocation, FRotator(), SpawnParams);
		
	}

}
void UCombatComponent::ServerSpawnKnifeActor_Implementation()
{
	if (Character && MeleeClass)
	{
		const FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(FName("KnifeSocket"));
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (!World) return;
		KnifeActor = World->SpawnActor<AMeleeKnife>(MeleeClass, SpawnLocation, FRotator(), SpawnParams);
	}
}
void UCombatComponent::MeleeFinished()
{
	
	UWorld* World = GetWorld();
	if (!World) return;
	World->DestroyActor(Cast<AActor>(KnifeActor));
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}
void UCombatComponent::SwapWeaponFinished()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		Character->bFinishedSwapping = true;
	}
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}
void UCombatComponent::CachePendingSwapWeapons()
{
	PendingSwapEquippedWeapon = SecondaryWeapon;
	PendingSwapSecondaryWeapon = EquippedWeapon;
}
void UCombatComponent::SwapAttachWeapons()
{
	//AWeapon* WeaponToSwap = EquippedWeapon;
	//EquippedWeapon = SecondaryWeapon;
	//SecondaryWeapon = WeaponToSwap;
	EquippedWeapon = PendingSwapEquippedWeapon;
	SecondaryWeapon = PendingSwapSecondaryWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateAmmoReserves();
	PlayEquipWeaponSound(EquippedWeapon);
	if (EquippedWeapon->AmmoIsEmpty()) ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquipSecondary);
	AttachActorToBackpack(SecondaryWeapon);
	
}
void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
	
}
int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMagazine = EquippedWeapon->GetAmmoMag() - EquippedWeapon->GetAmmo(); 
	if (AmmoReservesMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 ReservesAmount = AmmoReservesMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMagazine, ReservesAmount); //returns the smaller value of room in magazine and reserves amount and stores it in least
		return FMath::Clamp(RoomInMagazine, 0, Least); // room in mag will always return the same value as least
	}
	return 0;
}
void UCombatComponent::UpdateAmmoAmount()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (AmmoReservesMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		AmmoReservesMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		AmmoReserves = AmmoReservesMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDAmmoReserves(AmmoReserves);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}
void UCombatComponent::UpdateAmmoReserves()
{
	if (!EquippedWeapon) return;
	if (AmmoReservesMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		AmmoReserves = AmmoReservesMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDAmmoReserves(AmmoReserves);
		PlayerController->ShowWeaponIcon(EquippedWeapon->GetWeaponIcon());
	}
}

void UCombatComponent::UpdateShotgunAmmo()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (AmmoReservesMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		AmmoReservesMap[EquippedWeapon->GetWeaponType()] -= 1;
		AmmoReserves = AmmoReservesMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDAmmoReserves(AmmoReserves);
	}
	EquippedWeapon->AddAmmo(1);
	bCanShoot = true;
	if (EquippedWeapon->AmmoIsFull() || AmmoReserves == 0) // if full jump to the end of the reload animation
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::TraceCrosshairs(FHitResult& TraceHitResult)
{
	//Getting the viewport size 
	FVector2D Viewport;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Viewport);
	}
	//calculation to position the crosshair in the middle of the screen
	FVector2D CrosshairLocation(Viewport.X / 2.f, Viewport.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	//conversion from screen space to world space result stored in boolean
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		//setting the start and point for the line trace
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 90.f);

		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		//Line trace by single channel handles collision of the line trace for us. We just have to set the impact point to the end point of the trace if it returns false
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInterface>())
		{
			HUDPackage.CrosshairColour = FLinearColor::Red;
			TraceAgainstCharacter = true;
		}
		else
		{
			HUDPackage.CrosshairColour = FLinearColor::White;
		}
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			TraceAgainstCharacter = false;
		}

	}

}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	// checks if player controller is equal to null if true we cast our player controller to the controller inherited from Unreal's character class, 
	//if false we set it to the player controller
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Character->Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<AEverzoneHUD>(PlayerController->GetHUD()) : PlayerHUD;
		if (PlayerHUD)
		{
			if (EquippedWeapon)
			{

				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairDown = EquippedWeapon->CrosshairDown;
				HUDPackage.CrosshairUp = EquippedWeapon->CrosshairUp;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;

			}
			else
			{

				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairDown = nullptr;
				HUDPackage.CrosshairUp = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;

			}
			//getting the ranges and velocity needed for the multiplier value in GetMappedRangeValueClamped
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D CrouchWalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeedCrouched);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			//mapping player walkspeed from [0,600] to [0,1]
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			if (Character->bIsCrouched)
			{
				//mapping player walkspeed from [0,300] to [0,1]
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(CrouchWalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			}
			if (Character->GetCharacterMovement()->IsFalling())
			{
				//Interp used to smoothly transition between crosshair states
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if (bIsAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimInterpTarget, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			if (TraceAgainstCharacter && !bIsAiming)
			{
				CharacterTraceFactor = FMath::FInterpTo(CharacterTraceFactor, -0.2f, DeltaTime, 30.f);
			}
			else
			{
				CharacterTraceFactor = FMath::FInterpTo(CharacterTraceFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, 40.f);

			// The hard coded 0.3 float value here is needed to prevent the crosshairs from shrinking and overlapping each other which makes the crosshair look distorted
			HUDPackage.CrosshairSpread = 0.3f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CharacterTraceFactor + CrosshairShootFactor;

			PlayerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
	}
}


/*
* This is responsible for the camera zooming in and out when the player aims in and out
*/
void UCombatComponent::InterpFov(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bIsAiming)
	{
		CurrentFov = FMath::FInterpTo(CurrentFov, EquippedWeapon->ZoomedFOV, DeltaTime, EquippedWeapon->ZoomInterpSpd);
	}
	else
	{
		CurrentFov = FMath::FInterpTo(CurrentFov, DefaultFov, DeltaTime, EquippedWeapon->ZoomInterpSpd);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFov);
	}
}
