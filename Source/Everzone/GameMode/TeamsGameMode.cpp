// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Everzone/GameState/EverzoneGameState.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Kismet/GameplayStatics.h"

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

	if (!KillerPlayerState || !VictimPlayerState) return BaseDamage;

	if (VictimPlayerState == KillerPlayerState) return BaseDamage;

	if (VictimPlayerState->GetTeam() == KillerPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
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
