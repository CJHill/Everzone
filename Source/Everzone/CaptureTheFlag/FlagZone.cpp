// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Everzone/Weapon/Flag.h"
#include "Everzone/GameMode/CaptureTheFlagMode.h"

// Sets default values
AFlagZone::AFlagZone()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(FName("Zone Sphere"));
	SetRootComponent(ZoneSphere);
}

// Called when the game starts or when spawned
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);

	//if the flag belongs to the enemy team continue to executed code in the statement
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagMode* CTFGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagMode>();
		if (CTFGameMode)
		{
			// handles scoring points for the team
			CTFGameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}



