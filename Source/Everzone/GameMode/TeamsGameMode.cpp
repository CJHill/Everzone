// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Everzone/GameState/EverzoneGameState.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bIsTeamMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(UGameplayStatics::GetGameState(this));
	if (!EverzoneGameState)return;

	AEverzonePlayerState* EverzonePlayerState = NewPlayer->GetPlayerState<AEverzonePlayerState>();
	if (EverzonePlayerState && EverzonePlayerState->GetTeam() == ETeam::ET_NoTeam)
	{
		if (EverzoneGameState->BlueTeam.Num() >= EverzoneGameState->OrangeTeam.Num())
		{
			EverzoneGameState->OrangeTeam.AddUnique(EverzonePlayerState);
			EverzonePlayerState->SetTeam(ETeam::ET_OrangeTeam);
		}
		else
		{
			EverzoneGameState->BlueTeam.AddUnique(EverzonePlayerState);
			EverzonePlayerState->SetTeam(ETeam::ET_BlueTeam);
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(UGameplayStatics::GetGameState(this));
	if (!EverzoneGameState)return;

	AEverzonePlayerState* EverzonePlayerState = Exiting->GetPlayerState<AEverzonePlayerState>();
	if (!EverzonePlayerState) return;
	if (EverzoneGameState->OrangeTeam.Contains(EverzonePlayerState))
	{
		EverzoneGameState->OrangeTeam.Remove(EverzonePlayerState);
	}
	else if (EverzoneGameState->BlueTeam.Contains(EverzonePlayerState))
	{
		EverzoneGameState->BlueTeam.Remove(EverzonePlayerState);
	}
	
}

float ATeamsGameMode::CalculateDamage(AController* Killer, AController* Victim, float BaseDamage)
{
	if (!Killer || !Victim) return BaseDamage;

	AEverzonePlayerState* KillerPlayerState = Killer->GetPlayerState<AEverzonePlayerState>();
	AEverzonePlayerState* VictimPlayerState = Victim->GetPlayerState<AEverzonePlayerState>();

	if (!KillerPlayerState || !VictimPlayerState) return BaseDamage; // null check

	if (VictimPlayerState == KillerPlayerState) return BaseDamage; // if someone shoots themselves with a rocket launcher for example do that damage to the player

	if (VictimPlayerState->GetTeam() == KillerPlayerState->GetTeam()) //if someone shoots a teammate do 0 damage.
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController, AEverzonePlayerController* KillersController)
{
	Super::PlayerEliminated(PlayerKilled, VictimsController, KillersController);

	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(UGameplayStatics::GetGameState(this));
	AEverzonePlayerState* KillerPlayerState = KillersController ? Cast<AEverzonePlayerState>(KillersController->PlayerState) : nullptr;
	if (EverzoneGameState && KillerPlayerState && VictimsController != KillersController)
	{
		if (KillerPlayerState->GetTeam() == ETeam::ET_BlueTeam) // award point/s to the blue team
		{
			EverzoneGameState->BlueTeamScores();
		}
		if (KillerPlayerState->GetTeam() == ETeam::ET_OrangeTeam) // award point/s to the orange team
		{
			EverzoneGameState->OrangeTeamScores();
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(UGameplayStatics::GetGameState(this));

	if (!EverzoneGameState)return;

	for (auto& PlayerState : EverzoneGameState->PlayerArray)
	{
		AEverzonePlayerState* EverzonePlayerState = Cast<AEverzonePlayerState>(PlayerState.Get());
		if (EverzonePlayerState && EverzonePlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			// if there are more blue players than orange players or the same amount on each side assign the next player to orange. Else add the player to the blue team
			if (EverzoneGameState->BlueTeam.Num() >= EverzoneGameState->OrangeTeam.Num())
			{
				EverzoneGameState->OrangeTeam.AddUnique(EverzonePlayerState);
				EverzonePlayerState->SetTeam(ETeam::ET_OrangeTeam);
			}
			else
			{
				EverzoneGameState->BlueTeam.AddUnique(EverzonePlayerState);
				EverzonePlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}
