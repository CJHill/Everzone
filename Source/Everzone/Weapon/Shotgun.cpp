// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Shoot(const FVector& HitTarget)
{
	// we're calling the weapon shoot function to handle the basic functionality like playing the animation and handling the bullet shell without inheriting the unique functionality from the
	// Hit Scan Weapon class
	AWeapon::Shoot(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();

	TMap<AEverzoneCharacter*, uint32>  HitMap;
	for (uint32 i = 0; i < NumOfShotgunPellets; i++)
	{

		FHitResult WeaponHit;
		WeaponTraceHit(Start, HitTarget, WeaponHit);
		AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(WeaponHit.GetActor());
		if (EverzoneCharacter && HasAuthority() && InstigatorController)
		{
			if (HitMap.Contains(EverzoneCharacter))
			{
				HitMap[EverzoneCharacter]++;
			}
			else
			{
				HitMap.Emplace(EverzoneCharacter, 1);
			}
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, WeaponHit.ImpactPoint, WeaponHit.ImpactNormal.Rotation());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, WeaponHit.ImpactPoint, .3f, FMath::RandRange(-.5f, .5f));
		}
	}
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
		}
	}
}
