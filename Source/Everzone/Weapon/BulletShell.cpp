// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
// Sets default values
ABulletShell::ABulletShell()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Shell Mesh"));
	SetRootComponent(ShellMesh);
	ShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectImpulse = 8.f;
	
}


void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	ShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	ShellMesh->AddImpulse(GetActorForwardVector()* ShellEjectImpulse);
	SetLifeSpan(3.f);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	
}


void ABulletShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

