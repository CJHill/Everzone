// Fill out your copyright notice in the Description page of Project Settings.

#include "EverzoneCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Everzone/Weapon/Weapon.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"
#include "Everzone/EverzoneComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EverzoneAnimInstance.h"
#include "Everzone/Everzone.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Everzone/GameMode/EverzoneGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Everzone/Weapon/WeaponTypes.h"
#include "Everzone/HUD/EverzoneHUD.h"
#include "Everzone/HUD/OverlayWidget.h"
#include "Components/BoxComponent.h"


// Sets default values
AEverzoneCharacter::AEverzoneCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;// how far away camera is from player
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

	BuffComp = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));
	BuffComp->SetIsReplicated(true);

	LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("Lag Compensation Component"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Dissolve Timeline"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*Hit boxes for server side rewind*/
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head Hitbox"));
	Head->SetupAttachment(GetMesh(), FName("head"));
	Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis Hitbox"));
	Pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	Pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02 Hitbox"));
	Spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	Spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03 Hitbox"));
	Spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	Spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_l Hitbox"));
	Upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	Upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_r Hitbox"));
	Upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	Upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_l Hitbox"));
	Lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	Lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_r Hitbox"));
	Lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	Lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_l Hitbox"));
	Hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	Hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_r Hitbox"));
	Hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	Hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("Backpack Hitbox"));
	Backpack->SetupAttachment(GetMesh(), FName("backpack"));
	Backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Upperbackpack = CreateDefaultSubobject<UBoxComponent>(TEXT("Upper Backpack Hitbox"));
	Upperbackpack->SetupAttachment(GetMesh(), FName("blanket_r"));
	Upperbackpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_l Hitbox"));
	Thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	Thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_r Hitbox"));
	Thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	Thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_l Hitbox"));
	Calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	Calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_r Hitbox"));
	Calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	Calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_l Hitbox"));
	Foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	Foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_r Hitbox"));
	Foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	Foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEverzoneCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AEverzoneCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AEverzoneCharacter, CurrentHealth);
	DOREPLIFETIME(AEverzoneCharacter, Shield);
	DOREPLIFETIME(AEverzoneCharacter, bDisableGameplay);
}

void AEverzoneCharacter::Destroyed()
{
	Super::Destroyed();
	if (DeathBotComp)
	{
		DeathBotComp->DestroyComponent();
	}
	AEverzoneGameMode* EverzoneGameMode = Cast<AEverzoneGameMode>(UGameplayStatics::GetGameMode(this));
	bool bIsMatchNotInProgress = EverzoneGameMode && EverzoneGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComp && CombatComp->EquippedWeapon && bIsMatchNotInProgress)
	{
		CombatComp->EquippedWeapon->Destroy();
	}
}


void AEverzoneCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();

	if (PlayerController)
	{
		PlayerController->HideDeathMessage();
	}
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AEverzoneCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void AEverzoneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	HideCamera();
	GetAndInitHUD();
}
void AEverzoneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AEverzoneCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AEverzoneCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AEverzoneCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AEverzoneCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released , this, &AEverzoneCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AEverzoneCharacter::ShootButtonPressed);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AEverzoneCharacter::ShootButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AEverzoneCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AEverzoneCharacter::GrenadeButtonPressed);
	//PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &AEverzoneCharacter::MeleeButtonPressed);
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
	if (BuffComp)
	{
		BuffComp->Character = this;
		BuffComp->SetInitialSpd(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComp->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComp)
	{
		LagCompensationComp->Character = this;
		if (Controller)
		{
			LagCompensationComp->PlayerController = Cast<AEverzonePlayerController>(Controller);
		}
	}
}
void AEverzoneCharacter::PlayShootMontage(bool bAiming) 
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ShootMontage)
	{
		AnimInstance->Montage_Play(ShootMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AEverzoneCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		
	}
}
void AEverzoneCharacter::PlayReloadMontage()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComp->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rocket");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("Sniper");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Grenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEverzoneCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);

	}
}

void AEverzoneCharacter::PlayMeleeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MeleeMontage)
	{
		AnimInstance->Montage_Play(MeleeMontage);
	}
}

void AEverzoneCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxyRotate();
	
	TimeSinceLastSimReplication = 0.f;
}

void AEverzoneCharacter::Eliminated()
{
	HandleWeaponsOnDeath();

	if (PlayerController)
	{
		PlayerController->HideWeaponIcon();
	}
	MulticastEliminated();
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &AEverzoneCharacter::EliminatedTimerFinished, EliminatedDelay);
}

void AEverzoneCharacter::MulticastEliminated_Implementation()
{
	if (PlayerController)
	{
		PlayerController->SetHUDWeaponAmmo(0);

	}
	bIsEliminated = true;
	PlayElimMontage();
	//Changing the dissolve material to add the dissolve effects
	if (DissolveMatInst)
	{
		InstDynamicDissolveMat = UMaterialInstanceDynamic::Create(DissolveMatInst, this);
		GetMesh()->SetMaterial(0, InstDynamicDissolveMat);
		InstDynamicDissolveMat->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		InstDynamicDissolveMat->SetScalarParameterValue(TEXT("Glow"), 180.f);
	}
	StartDissolve();
	if (CombatComp)
	{
		CombatComp->ShootButtonPressed(false);

		bool bHideSniperScope = IsLocallyControlled() &&
		CombatComp->bIsAiming == true &&
		CombatComp->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;

		// if the character dies whilst aiming with a sniper hide the scope
		if (bHideSniperScope)
		{
			ShowSniperScope(false);
		}
	}
	//disable movement and collision
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn DeathBot
	if (DeathBotEffect)
	{
		FVector DeathBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		DeathBotComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathBotEffect, DeathBotSpawnPoint, GetActorRotation());
	}
	if (DeathBotCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, DeathBotCue, GetActorLocation());
	}

}

void AEverzoneCharacter::EliminatedTimerFinished()
{
	AEverzoneGameMode* EverzoneGameMode = GetWorld()->GetAuthGameMode<AEverzoneGameMode>(); // Use this line to get the game mode when you need it
	if (EverzoneGameMode)
	{
		EverzoneGameMode->RequestRespawn(this, PlayerController);
	}
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(this->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->HideDeathMessage();
	}
}
void AEverzoneCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}
void AEverzoneCharacter::HandleWeaponsOnDeath()
{
	if (!CombatComp) return;

	if (CombatComp->EquippedWeapon)
	{
		DropOrDestroyWeapon(CombatComp->EquippedWeapon);
	}
	if (CombatComp->SecondaryWeapon)
	{
		DropOrDestroyWeapon(CombatComp->SecondaryWeapon);
	}
}
void AEverzoneCharacter::UpdateDissolveMat(float DissolveMatValue)
{
	if (InstDynamicDissolveMat)
	{
		InstDynamicDissolveMat->SetScalarParameterValue(TEXT("Dissolve"), DissolveMatValue);
	}
}
void AEverzoneCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AEverzoneCharacter::UpdateDissolveMat);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}
void AEverzoneCharacter::PlayHitReactMontage()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AEverzoneCharacter::GetAndInitHUD()
{
	if (EverzonePlayerState == nullptr)
	{
		EverzonePlayerState = GetPlayerState<AEverzonePlayerState>();
		if (EverzonePlayerState)
		{
			//This is to refresh the count for the score and death properties nothing is being added here
			EverzonePlayerState->AddToPlayerScore(0.f);
			EverzonePlayerState->AddToPlayerDeaths(0);
			EverzonePlayerState->SetKillersName("");
		}
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
	if (bDisableGameplay) return;
	if (CombatComp)
	{
		ServerEquipButtonPressed();
	}
}

void AEverzoneCharacter::ServerEquipButtonPressed_Implementation()
{
	if (!CombatComp) return;
	
	if (OverlappingWeapon)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
	else if (CombatComp->bShouldSwapWeapons())
	{
		CombatComp->SwapWeapons();
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
void AEverzoneCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComp)
	{
		CombatComp->Reload();
	}
}
void AEverzoneCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
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
void AEverzoneCharacter::ShootButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComp)
	{
		CombatComp->ShootButtonPressed(true);
	}
}
void AEverzoneCharacter::ShootButtonReleased()
{
	if (bDisableGameplay) return;
	if (CombatComp)
	{
		CombatComp->ShootButtonPressed(false);
	}
}
void AEverzoneCharacter::GrenadeButtonPressed()
{
	if (CombatComp)
	{
		CombatComp->ThrowGrenade();
	}

}
//void AEverzoneCharacter::MeleeButtonPressed()
//{
//	if (CombatComp)
//	{
//		CombatComp->Melee();
//	}
//}
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
void AEverzoneCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bIsEliminated) return;
	float DamageToHealth = Damage;
	if (Shield > 0)
	{
		if (Shield >= Damage)// check to see if the shield can withstand the damage just received
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			// How much damage the shield has absorbed before emptying
			const float ShieldAbsorb = FMath::Clamp(DamageToHealth - Shield, 0.f, MaxShield);
			// Subtract damage from the Shield
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);
			//Subtract ShieldAbsorb from DamageToHealth. For example if Damage is 170 and shield is 100 the shield would absorb 100 leaving 70 remaining to do damamge to the player's health
			DamageToHealth -= FMath::Clamp(DamageToHealth - ShieldAbsorb, 0.f, Damage);
		}
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	if (CurrentHealth == 0.f)
	{
		AEverzoneGameMode* EverzoneGameMode = GetWorld()->GetAuthGameMode<AEverzoneGameMode>();
		if (EverzoneGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Controller) : PlayerController;
			AEverzonePlayerController* KillersController = Cast<AEverzonePlayerController>(InstigatorController);
			EverzoneGameMode->PlayerEliminated(this, PlayerController, KillersController);
		}
	}
	
}


void AEverzoneCharacter::AimOffset(float DeltaTime)
{
	if (CombatComp && CombatComp->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0 && !bIsInAir)//whilst standing still 
	{
		bRotateRootBone = true;
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
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AEverzoneCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map the pitch rotation from (270, 360) to (-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);

	}
}

float AEverzoneCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	
	return Velocity.Size();
}

void AEverzoneCharacter::RotateInPlace(float DeltaTime)
{
	// The order when creating enums matter by peeking the definition of net role you can see Sim proxy is considered lower or less than autonomus or authoritative as enums are assigned ints
	// so by checking if the local role is greater than simulated proxy AimOffset will only be called on the locally controlled client and the server
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastSimReplication += DeltaTime;
		if (TimeSinceLastSimReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AEverzoneCharacter::SimProxyRotate()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	SimProxyRotationLastFrame = SimProxyRotation;
	SimProxyRotation = GetActorRotation();
	SimProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(SimProxyRotation, SimProxyRotationLastFrame).Yaw;
	

	if (FMath::Abs(SimProxyYaw) > SimYawThreshold)
	{
		if (SimProxyYaw > SimYawThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (SimProxyYaw < -SimYawThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
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

void AEverzoneCharacter::HideCamera()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraTransition)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AEverzoneCharacter::OnRep_Health(float LastHealthValue)
{
	UpdateHUDHealth();
	if (CurrentHealth < LastHealthValue) //if check is to see if health has decreased, if so it means we've taken dmg so play the hit react montage
	{
		PlayHitReactMontage();
	}
	
}
void AEverzoneCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}
void AEverzoneCharacter::UpdateHUDShield()
{
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDShield(Shield, MaxShield);
	}
}
void AEverzoneCharacter::UpdateHUDAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Controller) : PlayerController;
	if (PlayerController && CombatComp && CombatComp->EquippedWeapon)
	{
		PlayerController->SetHUDAmmoReserves(CombatComp->AmmoReserves);
		PlayerController->SetHUDWeaponAmmo(CombatComp->EquippedWeapon->GetAmmo());
	}
}
void AEverzoneCharacter::SpawnDefaultWeapon()
{
	AEverzoneGameMode* EverzoneGameMode = Cast<AEverzoneGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (EverzoneGameMode && World && !bIsEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (CombatComp)
		{
			CombatComp->EquipWeapon(StartingWeapon);
			UpdateHUDAmmo();
		}
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->HideWeaponIcon();
		}
	}
}
void AEverzoneCharacter::OnRep_Shield(float LastShieldValue)
{
	UpdateHUDShield();
	if (Shield < LastShieldValue)
	{
		PlayHitReactMontage();
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

FVector AEverzoneCharacter::GetHitTarget() const
{
	if(CombatComp == nullptr) return FVector();
	return CombatComp->HitTarget;
}

ECombatState AEverzoneCharacter::GetCombatState() const
{
	if (CombatComp == nullptr) return ECombatState::ECS_MAX;
	return CombatComp->CombatState;
}

bool AEverzoneCharacter::IsLocallyReloading()
{
	if(!CombatComp) return false;
	return CombatComp->bIsLocallyReloading;
	
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






