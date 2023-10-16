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
void UCombatComponent::Shoot()
{
	if (bCanShoot)
	{
		bCanShoot = false;
		ServerShoot(HitTarget);
		if (EquippedWeapon)
		{
			CrosshairShootFactor = 0.9f;
		}
		StartShootTimer();
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

void UCombatComponent::StartShootTimer()
{
	if (EquippedWeapon == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(ShootTimer, this, &UCombatComponent::EndShootTimer, EquippedWeapon->ShootDelay);
}

void UCombatComponent::EndShootTimer()
{ 
	bCanShoot = true;
	if (bShootIsPressed && EquippedWeapon->bIsAutomatic)
	{
		
		Shoot();
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
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
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
			// The hard coded 0.3 float value is needed to prevent the crosshairs from shrinking and overlapping each other which makes the crosshair look distorted
			HUDPackage.CrosshairSpread = 0.3f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CharacterTraceFactor + CrosshairShootFactor;

			PlayerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

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
