// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"
#include "Kismet/GameplayStatics.h"
ARocketProjectile::ARocketProjectile()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ARocketProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* ShooterPawn = GetInstigator();
	if (!ShooterPawn) return;
	AController* ShooterController = ShooterPawn->GetController();
	if (ShooterController)
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
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

}
