// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Everzone/EverzoneComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->SetIsReplicated(true);
	ProjectileMovementComp->InitialSpeed = InitialSpeed;
	ProjectileMovementComp->MaxSpeed = InitialSpeed;
}
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	// Assign property name to the changed event's variable/property name. Assign None in case of null pointer
	FName PropertyName =Event.Property != nullptr ?  Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))// If property name is called Initial Speed
	{
		if (!ProjectileMovementComp) return;
		ProjectileMovementComp->InitialSpeed = InitialSpeed;
		ProjectileMovementComp->MaxSpeed = InitialSpeed;
	}
}
#endif
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AEverzoneCharacter* Character = Cast<AEverzoneCharacter>(GetOwner());
	if (!Character) return;
	
	AEverzonePlayerController* Controller = Cast<AEverzonePlayerController>(Character->Controller);
	if (!Controller) return;
	
	if (Character->HasAuthority() && !bUseServerSideRewind)
	{
		const float DamageToDeal = Hit.BoneName.ToString() == FString("head") ? HeadshotDamage : Damage;
		UGameplayStatics::ApplyDamage(OtherActor, DamageToDeal, Controller, this, UDamageType::StaticClass());
		
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
		return;
	}
	AEverzoneCharacter* HitCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (bUseServerSideRewind && Character->GetLagCompensationComp() && Character->IsLocallyControlled() && HitCharacter)
	{
		
		Character->GetLagCompensationComp()->ServerProjectileScoreRequest(HitCharacter, TraceStart, InitialVelocity, Controller->GetCurrentServerTime() - Controller-> SingleTripTime);
	}
			
		
	
	
	Super::OnHit(HitComp, OtherActor, OtherComp,NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	
}
