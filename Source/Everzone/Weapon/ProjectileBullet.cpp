// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		AController* Controller = Character->Controller;
		if (Controller)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, Controller, this, UDamageType::StaticClass());
			
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp,NormalImpulse, Hit);
}
