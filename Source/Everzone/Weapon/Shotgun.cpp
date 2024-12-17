// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Everzone/EverzoneComponents/LagCompensationComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"



void AShotgun::ShootShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Shoot(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();

	//HitMap: maps to players' character models to the number of times a trace hits a character model
	TMap<AEverzoneCharacter*, uint32>  HitMap;
	TMap<AEverzoneCharacter*, uint32> HeadshotMap;
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult WeaponHit;
		WeaponTraceHit(Start, HitTarget, WeaponHit);

		AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(WeaponHit.GetActor());
		if (EverzoneCharacter)
		{
			const bool bHeadshot = WeaponHit.BoneName.ToString() == FString("head");
			if (bHeadshot)
			{
				if (HeadshotMap.Contains(EverzoneCharacter)) HeadshotMap[EverzoneCharacter]++;
				else HeadshotMap.Emplace(EverzoneCharacter, 1);
			}
			else
			{
				if (HitMap.Contains(EverzoneCharacter)) HitMap[EverzoneCharacter]++;
				else HitMap.Emplace(EverzoneCharacter, 1);
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

	}
	// This array is needed for the function parameter of shotgun score request
	TArray<AEverzoneCharacter*> HitCharacters;

	//Maps amount of hits to damage
	TMap<AEverzoneCharacter*, float> DamageMap;
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key)
		{
			//calculation for bodyshot damage amount of hits multiplied by damage info stored in damage map
			DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);

			HitCharacters.AddUnique(HitPair.Key);
			
		}
	}
	
	for (auto HeadshotHitPair : HeadshotMap)
	{
		if (HeadshotHitPair.Key)
		{
			//calculation for headshot damage amount of hits multiplied by damage. info stored in damage map
			if (DamageMap.Contains(HeadshotHitPair.Key)) HeadshotMap[HeadshotHitPair.Key]+= HeadshotHitPair.Value * HeadshotDamage;
			else DamageMap.Emplace(HeadshotHitPair.Key, HeadshotHitPair.Value * HeadshotDamage);

			HitCharacters.AddUnique(HeadshotHitPair.Key);

		}
	}

	for (auto DamagePair : DamageMap)
	{
		if (DamagePair.Key && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(DamagePair.Key, //player that was hit
					DamagePair.Value,
					InstigatorController,
					this, UDamageType::StaticClass());
			}
		}
	}
	

	if (!HasAuthority() && bUseServerSideRewind && OwnerPawn->IsLocallyControlled())
	{
		
		EverzoneOwningCharacter = EverzoneOwningCharacter == nullptr ? Cast<AEverzoneCharacter>(OwnerPawn) : EverzoneOwningCharacter;
		EverzoneOwningController = EverzoneOwningController == nullptr ? Cast<AEverzonePlayerController>(InstigatorController) : EverzoneOwningController;

		
		if (!EverzoneOwningController || !EverzoneOwningCharacter || !EverzoneOwningCharacter->GetLagCompensationComp())  return;

		
			EverzoneOwningCharacter->GetLagCompensationComp()->ServerShotgunScoreRequest(
				HitCharacters,
				Start,
				HitTargets,
				EverzoneOwningController->GetCurrentServerTime() - EverzoneOwningController->SingleTripTime);
			
		
	}
}

void AShotgun::ShotgunTraceEndPointWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalised = (HitTarget - TraceStart).GetSafeNormal(); //distance between the crosshair hit target and the muzzle flash in a noramlised vector
	const FVector SphereCenter = TraceStart + ToTargetNormalised * DistanceToSphere; //calculation to extend the center of bullet radius outwards in the game's world

	
	
	for (uint32 i = 0; i < NumOfShotgunPellets; i++)
	{
		const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius); //gets a random point in the bullet sphere radius 
		const FVector EndLocation = SphereCenter + RandVector;
	    FVector ToEndLocation = EndLocation - TraceStart;
		ToEndLocation = TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size();
		HitTargets.Add(ToEndLocation);
	}
}
