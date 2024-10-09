// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"

AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty(); // This forces a refresh of the weapon mesh's render so the outline ffect will be applied 
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	
	//This is where collison and overlap events are being Set
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState)
	DOREPLIFETIME(AWeapon, Ammo)
}
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (EverzoneCharacter)
	{
		EverzoneCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (EverzoneCharacter)
	{
		EverzoneCharacter->SetOverlappingWeapon(nullptr);
	}
}


void AWeapon::OnRep_Ammo()
{
	EverzoneOwningCharacter = EverzoneOwningCharacter == nullptr ? Cast<AEverzoneCharacter>(GetOwner()) : EverzoneOwningCharacter;
	if (EverzoneOwningCharacter && EverzoneOwningCharacter->GetCombatComp() && EverzoneOwningCharacter->GetCombatComp()->IsShotgun() && AmmoIsFull())
	{
		EverzoneOwningCharacter->GetCombatComp()->JumpToShotgunEnd();
	}
	SetHUDAmmo();

}
void AWeapon::UseAmmo()
{
	Ammo = FMath::Clamp(Ammo -1, 0, AmmoMagazine);
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	EverzoneOwningCharacter = EverzoneOwningCharacter == nullptr ? Cast<AEverzoneCharacter>(GetOwner()) : EverzoneOwningCharacter;
	if (EverzoneOwningCharacter)
	{
		EverzoneOwningController = EverzoneOwningController == nullptr ? Cast<AEverzonePlayerController>(EverzoneOwningCharacter->Controller) : EverzoneOwningController;
		if (EverzoneOwningController)
		{
			EverzoneOwningController->SetHUDWeaponAmmo(Ammo);
			EverzoneOwningController->SetHUDAmmoReserves(AmmoMagazine);
		}
	}
}
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
	
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		HandleOnEquipped();
		break;
	case EWeaponState::EWS_EquipSecondary:
		HandleOnEquipSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		HandleOnDropped();
		break;
	}
}
void AWeapon::HandleOnEquipped()
{
	if (!WeaponMesh || !AreaSphere) return;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetHUDAmmo();
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		//This is to enable the strap physics for the SMG and prevent the strap with colliding with other objects
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
}
void AWeapon::HandleOnDropped()
{
	if (!WeaponMesh || !AreaSphere) return;
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

}
void AWeapon::HandleOnEquipSecondary()
{
	if (!WeaponMesh || !AreaSphere) return;
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();
}
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}



void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}


void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		EverzoneOwningCharacter = nullptr;
		EverzoneOwningController = nullptr;
	}
	else
	{
		EverzoneOwningCharacter = EverzoneOwningCharacter == nullptr ? Cast<AEverzoneCharacter>(Owner) : EverzoneOwningCharacter;
		if (EverzoneOwningCharacter && EverzoneOwningCharacter->GetEquippedWeapon() && EverzoneOwningCharacter->GetEquippedWeapon() == this)
		{
           SetHUDAmmo();
		}
		
	}
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DroppedRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DroppedRules);
	SetOwner(nullptr);
	EverzoneOwningCharacter = nullptr;
	EverzoneOwningController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, AmmoMagazine);
	SetHUDAmmo();
}

bool AWeapon::AmmoIsEmpty()
{
	return Ammo <= 0;
}
bool AWeapon::AmmoIsFull()
{
	return Ammo == AmmoMagazine;
}

void AWeapon::Shoot(const FVector& HitTarget)
{
	if (ShootAnim)
	{
		WeaponMesh->PlayAnimation(ShootAnim, false);
	}
	if (BulletShellClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			
			UWorld* World = GetWorld();
			World->SpawnActor<ABulletShell>();
			if (World)
			{
				World->SpawnActor<ABulletShell>(BulletShellClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
			
		}
	}
	if (HasAuthority())
	{
		UseAmmo();
	}
	
}

