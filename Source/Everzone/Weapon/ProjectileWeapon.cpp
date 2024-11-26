// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Shoot(const FVector& HitTarget)
{
	Super::Shoot(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (!MuzzleFlashSocket || !World) return;

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation(); //from muzzle flash socket to hit location from TraceCrosshairs function
	FRotator TargetRotation = ToTarget.Rotation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	AProjectile* SpawnedProjectile = nullptr;
	if (bUseServerSideRewind)
	{
		if (InstigatorPawn->HasAuthority())// We're on the server here
		{
			if (InstigatorPawn->IsLocallyControlled())// Server Host player uses replicated projectile
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
			else // Server, not locally controlled. we spawn a non replicated projectile with no server side rewind as that client will fire their own server side rewind projectile
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
		else // On client machine using server side rewind
		{
			if (InstigatorPawn->IsLocallyControlled()) // player controlled by said client spawn a non replicated projectile that uses server side rewind
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnedProjectile->InitialVelocity = GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				SpawnedProjectile->Damage = Damage;
			}
			else //On client machine but a non locally controlled player pawn a non replicated projectile with no server side rewind
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}
	else // Weapon not using server side rewind
	{
		if (InstigatorPawn->HasAuthority())
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			SpawnedProjectile->bUseServerSideRewind = false;
			SpawnedProjectile->Damage = Damage;
		}
	}

}
