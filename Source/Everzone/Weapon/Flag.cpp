// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Flag Mesh"));
	SetRootComponent(FlagMesh);

	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);

	FlagMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();

}

void AFlag::Dropped()
{
	if (!FlagMesh) return;

	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DroppedRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DroppedRules);
	SetOwner(nullptr);
	EverzoneOwningCharacter = nullptr;
	EverzoneOwningController = nullptr;
}

void AFlag::ResetFlag()
{
	AEverzoneCharacter* FlagHolder = Cast<AEverzoneCharacter>(GetOwner());
	UCombatComponent* CombatComponent = Cast<UCombatComponent>(FlagHolder->GetCombatComp());
	if (FlagHolder)
	{
		FlagHolder->SetHoldingTheFlag(false);
		FlagHolder->SetOverlappingWeapon(false);
		FlagHolder->GetCharacterMovement()->MaxWalkSpeed = CombatComponent->GetBaseWalkSpeed();
	}
	if (!HasAuthority()) return;
	FDetachmentTransformRules DroppedRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DroppedRules);
	SetOwner(nullptr);
	EverzoneOwningCharacter = nullptr;
	EverzoneOwningController = nullptr;
	SetWeaponState(EWeaponState::EWS_Initial);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SetActorTransform(InitialTransform);
}

void AFlag::HandleOnEquipped()
{
	if (!FlagMesh || !GetAreaSphere()) return;
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECR_Overlap);

	EnableCustomDepth(false);
}

void AFlag::HandleOnDropped()
{
	if (!FlagMesh || !GetAreaSphere()) return;

	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

}




