// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Everzone/EverzoneComponents/LagCompensationComponent.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"
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
	

	FHitResult ShootResult;
	WeaponTraceHit(Start, HitTarget, ShootResult);

	AEverzoneCharacter* HitCharacter = Cast<AEverzoneCharacter>(ShootResult.GetActor()); 
	if (HitCharacter  && InstigatorController)
	{
	
		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
		if (HasAuthority() && bCauseAuthDamage)
		{
			const float DamageToDeal = ShootResult.BoneName.ToString() == FString("head") ? HeadshotDamage : Damage;
			
			UGameplayStatics::ApplyDamage(HitCharacter, DamageToDeal, InstigatorController, this, UDamageType::StaticClass());
		}
		if(!HasAuthority() && bUseServerSideRewind)
		{
			EverzoneOwningCharacter = EverzoneOwningCharacter == nullptr ? Cast<AEverzoneCharacter>(OwnerPawn) : EverzoneOwningCharacter;
			EverzoneOwningController = EverzoneOwningController == nullptr ? Cast<AEverzonePlayerController>(InstigatorController) : EverzoneOwningController;
			if (!EverzoneOwningCharacter || !EverzoneOwningCharacter->IsLocallyControlled() || !EverzoneOwningController || !EverzoneOwningCharacter->GetLagCompensationComp())  return;
			EverzoneOwningCharacter->GetLagCompensationComp()->ServerScoreRequest(
				HitCharacter,
				Start, 
				HitTarget, 
				EverzoneOwningController->GetCurrentServerTime() - EverzoneOwningController->SingleTripTime);
		}
		
	}
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, ShootResult.ImpactPoint, ShootResult.ImpactNormal.Rotation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ShootResult.ImpactPoint);
	}
	
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
	}
	if (ShootSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ShootSound, GetActorLocation());
	}
	
}



void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutputHit)
{
	
	UWorld* World = GetWorld();
	if (!World) return;
	FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f; 
	World->LineTraceSingleByChannel(OutputHit,
		TraceStart,
		End,
		ECollisionChannel::ECC_Visibility);
	FVector BeamEnd = End;
	if (OutputHit.bBlockingHit)
	{
		BeamEnd = OutputHit.ImpactPoint;
	}
	else
	{
		OutputHit.ImpactPoint = End;
	}
	
	if (BeamParticles)
	{
		UParticleSystemComponent* BeamParticlesComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true);
		if (BeamParticlesComp)
		{
			BeamParticlesComp->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}
