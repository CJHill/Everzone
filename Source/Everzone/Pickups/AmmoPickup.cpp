// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"
void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (EverzoneCharacter)
	{
		UCombatComponent* CombatComp = EverzoneCharacter->GetCombatComp();
		if (!CombatComp) return;
		CombatComp->PickupAmmo(WeaponType, AmountOfAmmo);
	}
	Destroy();
}
