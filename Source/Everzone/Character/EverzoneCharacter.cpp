// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Everzone/Weapon/Weapon.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEverzoneCharacter::AEverzoneCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	//Combat component will contain variables that need replicating so it's important that the component itself is also replicated
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	CombatComp->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AEverzoneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AEverzoneCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AEverzoneCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
void AEverzoneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
	
	
}
void AEverzoneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AEverzoneCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AEverzoneCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AEverzoneCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AEverzoneCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released , this, &AEverzoneCharacter::AimButtonReleased);
	PlayerInputComponent->BindAxis("MoveForward", this, &AEverzoneCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEverzoneCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AEverzoneCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AEverzoneCharacter::LookUp);

}
void AEverzoneCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComp)
	{
		CombatComp->Character = this;
	}
}
void AEverzoneCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		//creating a rotation matrix by passing in a FRotator which then gets and returns the X axis
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, value);
	}
}

void AEverzoneCharacter::MoveRight(float value)
{
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	AddMovementInput(Direction, value);
}

void AEverzoneCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void AEverzoneCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}
void AEverzoneCharacter::EquipButtonPressed()
{
	if (CombatComp)
	{
		
		if (HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AEverzoneCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}
void AEverzoneCharacter::CrouchButtonPressed()
{
	if (bIsCrouched) 
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}
void AEverzoneCharacter::AimButtonPressed()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(true);
	}
}
void AEverzoneCharacter::AimButtonReleased()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(false);
	}
}
void AEverzoneCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
void AEverzoneCharacter::AimOffset(float DeltaTime)
{
	if (CombatComp && CombatComp->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0 && !bIsInAir)//whilst standing still 
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0 || bIsInAir)// whilst running or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map the pitch rotation from (270, 360) to (-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	
	}
}

void AEverzoneCharacter::TurnInPlace(float DeltaTime)
{
	
	
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
		
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
		
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 10.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;

			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
void AEverzoneCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool AEverzoneCharacter::IsWeaponEquipped()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool AEverzoneCharacter::IsAiming()
{
	return (CombatComp && CombatComp->bIsAiming);
}

AWeapon* AEverzoneCharacter::GetEquippedWeapon()
{
	if (CombatComp == nullptr) return nullptr;
	return CombatComp->EquippedWeapon;
}

void AEverzoneCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}






