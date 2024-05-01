// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzonePlayerState.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Net/UnrealNetwork.h"

AEverzonePlayerState::AEverzonePlayerState()
{
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AEverzonePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEverzonePlayerState, Deaths);
	DOREPLIFETIME(AEverzonePlayerState, KilledBy);
}

void AEverzonePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}

void AEverzonePlayerState::OnRep_Deaths()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDeaths(Deaths);
		}
	}
}

void AEverzonePlayerState::OnRep_KilledBy()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->ShowDeathMessage(KilledBy);
		}
	}
}

void AEverzonePlayerState::AddToPlayerScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}

void AEverzonePlayerState::AddToPlayerDeaths(int32 DeathAmount)
{
	Deaths += DeathAmount;
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDeaths(Deaths);
		}
	}
}

void AEverzonePlayerState::SetKillersName(FString KillersName)
{
	KilledBy = KillersName;
	UpdateDeathMessage();
}

void AEverzonePlayerState::UpdateDeathMessage()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AEverzoneCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AEverzonePlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->ShowDeathMessage(KilledBy);
		}
	}
}
