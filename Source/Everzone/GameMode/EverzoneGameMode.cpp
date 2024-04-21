// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneGameMode.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AEverzoneGameMode::PlayerEliminated(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController, AEverzonePlayerController* KillersController)
{
	if (PlayerKilled)
	{
		PlayerKilled->Eliminated();
	}
}

void AEverzoneGameMode::RequestRespawn(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController)
{
	if (PlayerKilled)
	{
		PlayerKilled->Reset();
		PlayerKilled->Destroy();
	}
	if (VictimsController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(VictimsController, PlayerStarts[Selection]);
	}
}
