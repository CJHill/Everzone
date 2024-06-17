// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeKnife.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Everzone/Everzone.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

// look at hitscan weapon trace hit for implementing dmg for knife
AMeleeKnife::AMeleeKnife()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	KnifeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Knife Mesh"));
	KnifeMesh->SetupAttachment(RootComponent);
	SetRootComponent(KnifeMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
}

void AMeleeKnife::MulticastOnHit_Implementation(AEverzoneCharacter* HitPlayer)
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

// Called when the game starts or when spawned
void AMeleeKnife::BeginPlay()
{
	Super::BeginPlay();
	MeleeTraceHit();
	if (!MeleeHit.GetActor()) return;
	KnifeDamage(MeleeHit.GetActor());
}

void  AMeleeKnife::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
	UE_LOG(LogTemp, Warning, TEXT("Character Hit"));
	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(OtherActor);
	if (Character && Character->Implements<UCrosshairInterface>())
	{


		MulticastOnHit(EverzoneCharacter);

	}
	else
	{
		MulticastOnHit(nullptr);
	}
}


void AMeleeKnife::KnifeDamage(AActor* OtherActor)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	if (InstigatorController && HasAuthority())
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, InstigatorController, this, UDamageType::StaticClass());
	}
}

void AMeleeKnife::MeleeTraceHit()
{
	
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = GetActorLocation() + GetActorForwardVector() * 50.0f;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this->Owner);
	UWorld* World = GetWorld();
	if (!World) return;
	World->LineTraceSingleByChannel(MeleeHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	DrawDebugLine(World, TraceStart, TraceEnd, MeleeHit.bBlockingHit ? FColor::Blue : FColor::Red, false, 5.0f, 0, 10.0f);
	if (MeleeHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trace Hit Actor!"));
	}
}


