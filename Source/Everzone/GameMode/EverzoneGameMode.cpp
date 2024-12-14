// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneGameMode.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Everzone/GameState/EverzoneGameState.h"
namespace MatchState
{
	const FName CooldownState = FName(TEXT("Cooldown"));
}
AEverzoneGameMode::AEverzoneGameMode()
{
	bDelayedStart = true;
}
void AEverzoneGameMode::BeginPlay()
{
	
	Super::BeginPlay();
	LevelStartTime = GetWorld()->GetTimeSeconds();
	
}
void AEverzoneGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	//iterates through every controller in game and handles the match state functionality depending on what stage the game is at
	for (FConstPlayerControllerIterator ControllerIt = GetWorld()->GetPlayerControllerIterator(); ControllerIt; ++ControllerIt)
	{
		AEverzonePlayerController* EverzonePlayerController = Cast<AEverzonePlayerController>(*ControllerIt);
		if (EverzonePlayerController)
		{
			EverzonePlayerController->OnMatchStateSet(MatchState);
		}
	}
}
void AEverzoneGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::CooldownState);
		}
	}
	else if (MatchState == MatchState::CooldownState)
	{
		CountdownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}



void AEverzoneGameMode::PlayerEliminated(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController, AEverzonePlayerController* KillersController)
{
	AEverzoneGameState* EverzoneGameState = GetGameState<AEverzoneGameState>();
	AEverzonePlayerState* KillersPlayerState = KillersController ? Cast<AEverzonePlayerState>(KillersController->PlayerState) : nullptr;//Person who got the kill
	AEverzonePlayerState* VictimsPlayerState = VictimsController ? Cast<AEverzonePlayerState>(VictimsController->PlayerState) : nullptr;//Person who died

	if (KillersPlayerState && KillersPlayerState != VictimsPlayerState && EverzoneGameState)
	{
		//Caching the current top players
		TArray<AEverzonePlayerState*> PreviousLeadingPlayers = EverzoneGameState->TopScoringPlayers; 

		//The player who got the kill add 1 to their score and update the top scorer's array
		KillersPlayerState->AddToPlayerScore(1.0f);
		EverzoneGameState->UpdateTopScorer(KillersPlayerState);

		//Check if the player who got the kill is now in the top scoring players array if so call the crown function
		if (EverzoneGameState->TopScoringPlayers.Contains(KillersPlayerState))
		{
			AEverzoneCharacter* Leader = Cast<AEverzoneCharacter>(KillersPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		//notify previous top scoring players if they lost they lead
		for (AEverzonePlayerState* PreviousLeadPlayer : PreviousLeadingPlayers)
		{
			if (!EverzoneGameState->TopScoringPlayers.Contains(PreviousLeadPlayer))
			{
				AEverzoneCharacter* OldLeader = Cast<AEverzoneCharacter>(PreviousLeadPlayer->GetPawn());
				if (OldLeader)
				{
					OldLeader->MulticastLostTheLead();
				}
			}
		}

		//Notify players if they weren't previously leading the match
		for (AEverzonePlayerState* NewLeadPlayer : EverzoneGameState->TopScoringPlayers)
		{
			if (!PreviousLeadingPlayers.Contains(NewLeadPlayer))
			{
				AEverzoneCharacter* NewLeader = Cast<AEverzoneCharacter>(NewLeadPlayer->GetPawn());
				if (NewLeader)
				{
					NewLeader->MulticastGainedTheLead();
				}
			}
		}
		/*for (int32 i = 0; i < LeadingPlayers.Num(); i++)
		{
			if (EverzoneGameState->TopScoringPlayers.Contains(LeadingPlayers[i]))
			{
				AEverzoneCharacter* OldLeader = Cast<AEverzoneCharacter>(LeadingPlayers[i]->GetPawn());
				if (OldLeader)
				{
					OldLeader->MulticastLostTheLead();
				}
			}
		}*/

	}
	if (VictimsPlayerState && KillersPlayerState)
	{
		VictimsPlayerState->AddToPlayerDeaths(1);
		FString KillersName = KillersPlayerState->GetPlayerName();
		VictimsPlayerState->SetKillersName(KillersName);
	}
	if (PlayerKilled)
	{
		PlayerKilled->Eliminated(false);
		
		
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
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); // We minus one to ensure that the maximum value doesn't fall outside of the array index
		VictimsController->HideDeathMessage();
		RestartPlayerAtPlayerStart(VictimsController, PlayerStarts[Selection]);
	}
}

void AEverzoneGameMode::PlayerLeftGame(class AEverzonePlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	AEverzoneGameState* EverzoneGameState = GetGameState<AEverzoneGameState>();
	if (EverzoneGameState && EverzoneGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		EverzoneGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AEverzoneCharacter* CharacterLeaving = Cast<AEverzoneCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Eliminated(true);
	}
}


