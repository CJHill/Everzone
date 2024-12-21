// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneGameState.h"
#include "Net/UnrealNetwork.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
void AEverzoneGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEverzoneGameState, TopScoringPlayers);
	DOREPLIFETIME(AEverzoneGameState, OrangeTeamScore);
	DOREPLIFETIME(AEverzoneGameState, BlueTeamScore);
}

void AEverzoneGameState::UpdateTopScorer(AEverzonePlayerState* TopPlayer)
{
	if (TopScoringPlayers.Num() == 0) // if array is empty add the top player to the array and initialise the game's top score to that player's score
	{
		TopScoringPlayers.Add(TopPlayer);
		TopScore = TopPlayer->GetScore();
	}
	else if (TopPlayer->GetScore() == TopScore) // if another player is tied for the top spot add them to the array
	{
		TopScoringPlayers.AddUnique(TopPlayer);
	}
	else if (TopPlayer->GetScore() > TopScore) // if another player exceeds the current top score empty the array and add the new user to array and set their score to the game's top score
	{
      TopScoringPlayers.Empty();
	  TopScoringPlayers.AddUnique(TopPlayer);
	  TopScore = TopPlayer->GetScore();
	}
}

void AEverzoneGameState::OrangeTeamScores()
{
	++OrangeTeamScore;
	AEverzonePlayerController* OrangeController = Cast<AEverzonePlayerController>(GetWorld()->GetFirstPlayerController());
	if (OrangeController)
	{
		OrangeController->SetHUDOrangeTeamScores(OrangeTeamScore);
	}
}

void AEverzoneGameState::BlueTeamScores()
{
	++BlueTeamScore;
	AEverzonePlayerController* BlueController = Cast<AEverzonePlayerController>(GetWorld()->GetFirstPlayerController());
	if (BlueController)
	{
		BlueController->SetHUDBlueTeamScores(BlueTeamScore);
	}
}

void AEverzoneGameState::OnRep_OrangeTeamScore()
{
	AEverzonePlayerController* OrangeController = Cast<AEverzonePlayerController>(GetWorld()->GetFirstPlayerController());
	if (OrangeController)
	{
		OrangeController->SetHUDOrangeTeamScores(OrangeTeamScore);
	}
}

void AEverzoneGameState::OnRep_BlueTeamScore()
{
	AEverzonePlayerController* BlueController = Cast<AEverzonePlayerController>(GetWorld()->GetFirstPlayerController());
	if (BlueController)
	{
		BlueController->SetHUDBlueTeamScores(BlueTeamScore);
	}
}
