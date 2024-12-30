// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagMode.h"
#include "Everzone/CaptureTheFlag/FlagZone.h"
#include "Everzone/Weapon/Flag.h"
#include "Everzone/GameState/EverzoneGameState.h"


void ACaptureTheFlagMode::PlayerEliminated(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController, AEverzonePlayerController* KillersController)
{
	AEverzoneGameMode::PlayerEliminated(PlayerKilled, VictimsController, KillersController);

}

void ACaptureTheFlagMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->Team;
	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(GameState);
	if (!EverzoneGameState) return;

	if (FlagZone->Team == ETeam::ET_BlueTeam)
	{
		EverzoneGameState->BlueTeamScores();
	}
	if (FlagZone->Team == ETeam::ET_OrangeTeam)
	{
		EverzoneGameState->OrangeTeamScores();
	}
}
