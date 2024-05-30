// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/Everzone.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
// Sets default values
AProjectile::AProjectile()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (Tracer)
	{
		TracerComp = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}
	// We have to use this authority check because multicasts have to be called on the server to invoke to all clients including the server
	// subsequently by including the destroy function in OnHit() the super version of the Destroyed() function is also called on the server which will broadcast 
	// the information of the projectile being destroyed to all clients
	if (HasAuthority()) 
	{
		
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AEverzoneCharacter* Character = Cast<AEverzoneCharacter>(OtherActor);
	if (Character && Character->Implements<UCrosshairInterface>())
	{
		
	
		MulticastOnHit(Character);
	
	}
	else 
	{
		MulticastOnHit(nullptr);
	}
	
	Destroy();
}

void AProjectile::ExplosionDamage()
{
	
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
}

void AProjectile::SpawnProjectileTrail()
{
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
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}
void AProjectile::Destroyed()
{
	Super::Destroyed();
	
}

void AProjectile::MulticastOnHit_Implementation(AEverzoneCharacter* HitPlayer)
{
	if (HitPlayer)
	{
		if (PlayerImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, GetActorTransform());
		}
	}
	else 
	{
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
		}
	}
	
	
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

