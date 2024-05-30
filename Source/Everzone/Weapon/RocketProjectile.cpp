// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "RocketProjectileMovementComp.h"
ARocketProjectile::ARocketProjectile()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComp = CreateDefaultSubobject<URocketProjectileMovementComp>(TEXT("Rocket Movement Component"));
	RocketMovementComp->bRotationFollowsVelocity = true;
	RocketMovementComp->SetIsReplicated(true);
}
void ARocketProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{

		CollisionBox->OnComponentHit.AddDynamic(this, &ARocketProjectile::OnHit);

	}

	SpawnProjectileTrail();

	if (RocketLoop && RocketLoop_Att)
	{
		RocketLoopComp = UGameplayStatics::SpawnSoundAttached(RocketLoop, //SoundCue for rocket loop
			GetRootComponent(), // Attachment to Rocket Mesh
			FName(), // Empty as there is no socket to attach to
			GetActorLocation(), //Spawns at the same location as rocket mesh
			EAttachLocation::KeepWorldPosition, // World Transfrom remains the same
			false, // stop when attachment is destroyed. This is turned off
			1.f, // Volume multiplier
			1.f, // pitch multiplier
			0.f, // Start Time plays sound from the beginning of sound cue
			RocketLoop_Att, // rocket loop attenuation
			(USoundConcurrency*)nullptr, // we dont need this setting so its set to null
			false); // auto destroy is turned off
	}
}
void ARocketProjectile::Destroyed()
{

}

void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	ExplosionDamage();
	StartDestroyTimer();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (NSComp_SmokeTrail && NSComp_SmokeTrail->GetSystemInstanceController())
	{
		NSComp_SmokeTrail->GetSystemInstanceController()->Deactivate();

	}
	if (RocketLoopComp && RocketLoopComp->IsPlaying())
	{
		RocketLoopComp->Stop();
	}
}


