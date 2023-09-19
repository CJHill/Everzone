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
	ShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);// prevents camera jitter when bullets are in field of view
	ShellMesh->SetSimulatePhysics(true); 
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectImpulse = 8.f;
	
}


void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	ShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	FVector ShellVector = GetActorForwardVector();
	ShellVector.X = FMath::RandRange(-0.2f, 0.2f);
	ShellVector.Y = FMath::RandRange(-0.2f, 0.2f);
	ShellVector.Z = FMath::RandRange(-0.2f, 0.2f);
	ShellMesh->AddImpulse(ShellVector* (FMath::RandRange(2.f, 2.f) + ShellEjectImpulse)); //Ejects the bullet along the X axis by multiplying it's length by the Eject Impulse variable
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

