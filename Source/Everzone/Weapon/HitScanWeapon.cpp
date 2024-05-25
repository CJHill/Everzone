// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
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

	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(ShootResult.GetActor());
	if (EverzoneCharacter && HasAuthority() && InstigatorController)
	{
		UGameplayStatics::ApplyDamage(EverzoneCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
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

FVector AHitScanWeapon::TraceEndPointWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalised = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalised * DistanceToSphere;
	FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	FVector EndLocation = SphereCenter + RandVector;
	FVector ToEndLocation = EndLocation - TraceStart;
	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 5.f, 12, FColor::Cyan, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::Black, true);*/
	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutputHit)
{
	
	UWorld* World = GetWorld();
	if (!World) return;
	FVector End = bUseScatter ? TraceEndPointWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
	World->LineTraceSingleByChannel(OutputHit,
		TraceStart,
		End,
		ECollisionChannel::ECC_Visibility);
	FVector BeamEnd = End;
	if (OutputHit.bBlockingHit)
	{
		BeamEnd = OutputHit.ImpactPoint;
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
