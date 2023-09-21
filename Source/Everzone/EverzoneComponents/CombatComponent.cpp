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
#include "Everzone/HUD/EverzoneHUD.h"
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
	}
	
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshair(DeltaTime);
}

void UCombatComponent::SetAiming(bool bAiming)
{
	bIsAiming = bAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::ShootButtonPressed(bool bIsPressed)
{
	 bShootIsPressed = bIsPressed;
	 if (bShootIsPressed)
	 {
		 FHitResult HitResult;
		 TraceCrosshairs(HitResult);
		 ServerShoot(HitResult.ImpactPoint);
	 }
}

void UCombatComponent::TraceCrosshairs(FHitResult& TraceHitResult)
{
	//Getting the viewport
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
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		//Line trace by single channel handles collision of the line trace for us. We just have to set the impact point to the end point of the trace if it returns false
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility); 
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			 
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
			FHUDPackage HUDPackage;
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
			PlayerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::ServerShoot_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastShoot(TraceHitTarget);
}

void UCombatComponent::MulticastShoot_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayShootMontage(bIsAiming);
		EquippedWeapon->Shoot(TraceHitTarget);
	}
}
/* Checks to see if the character and weapon to equip variable is equal to null if it is then it calls return.Otherwise it changes the weapon state to equipped
* Gets the hand socket the from the characters skeleton in the editor
* lastly gives ownership of the equipped weapon to the character in possession of the weapon and hides the pick up widget from display
 */
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

