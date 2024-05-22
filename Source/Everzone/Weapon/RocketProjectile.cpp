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
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	if (NS_SmokeTrail)
	{
		NSComp_SmokeTrail = UNiagaraFunctionLibrary::SpawnSystemAttached(NS_SmokeTrail, //Niagara System
			GetRootComponent(), // Attachment to Rocket Mesh
			FName(), // Empty as there is no socket to attach to
			GetActorLocation(), //Spawns at the same location as rocket mesh
			GetActorRotation(), //spawns at the same rotation as rocket mesh
			EAttachLocation::KeepWorldPosition, // World Transfrom remains the same
			false); // Auto destroy set to false as we want the FX to linger slightly after the impact of the projectile
	}
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
void ARocketProjectile::DestroyTimerFinished()
{
	Destroy();
}
void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	APawn* ShooterPawn = GetInstigator();
	if (!ShooterPawn) return;
	AController* ShooterController = ShooterPawn->GetController();
	if (ShooterController && HasAuthority())
	{
		UGameplayStatics::ApplyRadialDamageWithFalloff(this, // World Context object
			Damage,
			10.f,// Minimum damage
			GetActorLocation(), //Epicenter of damage
			InnerDamageRadius, OuterDamageRadius,
			1.f, // Damage Fall off 
			UDamageType::StaticClass(), //DamageType Class
			TArray<AActor*>(),  //Actors to ignore
			this, // damage causer
			ShooterController); // Controller of Instigator
	}
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ARocketProjectile::DestroyTimerFinished, DestroyTime);
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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


