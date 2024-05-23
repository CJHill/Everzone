// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
void AHitScanWeapon::Shoot(const FVector& HitTarget)
{
	Super::Shoot(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	FVector End = Start + (HitTarget - Start) * 1.25f; // Extending the end point just enough so that the Hit Target will always be a blocking hit

	FHitResult ShootResult;
	UWorld* World = GetWorld();
	if (!World) return;
	
	World->LineTraceSingleByChannel(ShootResult,
	Start,
	End,
	ECollisionChannel::ECC_Visibility);
	FVector BeamEnd = End;
	if (BeamParticles)
	{
		UParticleSystemComponent* BeamParticlesComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform);
		if (BeamParticlesComp)
		{
			BeamParticlesComp->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
	if (ShootResult.bBlockingHit)
	{
		BeamEnd = ShootResult.ImpactPoint;
		AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(ShootResult.GetActor());
		if (EverzoneCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(EverzoneCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, ShootResult.ImpactPoint, ShootResult.ImpactNormal.Rotation());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ShootResult.ImpactPoint);
		}
	}
	
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
	}
	if (ShootSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ShootSound, GetActorLocation());
	}
	
}
