// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/EverzoneComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
	PickupEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Pickup Effect Component"));
	PickupEffectComp->SetupAttachment(RootComponent);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (!EverzoneCharacter) return;
	
	UBuffComponent* Buff = EverzoneCharacter->GetBuffComp();
	if (Buff)
	{
		Buff->Heal(HealAmount, HealOverTime);
	}
	
	Destroy();
}

void AHealthPickup::Destroyed()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(), GetActorRotation());
	}
	
	Super::Destroyed();
}


