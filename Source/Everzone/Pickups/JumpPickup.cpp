// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/EverzoneComponents/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (!EverzoneCharacter) return;

	UBuffComponent* Buff = EverzoneCharacter->GetBuffComp();
	if (Buff)
	{
		Buff->JumpBuff(JumpVelocityZBuff, JumpBuffTime);
	}

	Destroy();
}
