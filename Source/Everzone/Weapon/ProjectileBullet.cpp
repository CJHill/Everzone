// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
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

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	/*FPredictProjectilePathParams ProjectilePathParams;
	ProjectilePathParams.ActorsToIgnore.Add(this);
	ProjectilePathParams.bTraceWithChannel = true;
	ProjectilePathParams.bTraceWithCollision = true;
	ProjectilePathParams.DrawDebugTime = 5.f;
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	ProjectilePathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	ProjectilePathParams.MaxSimTime = 4.f;
	ProjectilePathParams.ProjectileRadius = 5.f;
	ProjectilePathParams.SimFrequency = 10.f;
	ProjectilePathParams.StartLocation = GetActorLocation();
	ProjectilePathParams.TraceChannel = ECollisionChannel::ECC_Visibility;

	FPredictProjectilePathResult ProjectilePathResult;
	UGameplayStatics::PredictProjectilePath(this, ProjectilePathParams, ProjectilePathResult);*/
}
