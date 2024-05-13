// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneGameMode.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/PlayerController/EverzonePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Net/UnrealNetwork.h"

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
}



void AEverzoneGameMode::PlayerEliminated(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController, AEverzonePlayerController* KillersController)
{
	AEverzonePlayerState* KillersPlayerState = KillersController ? Cast<AEverzonePlayerState>(KillersController->PlayerState) : nullptr;
	AEverzonePlayerState* VictimsPlayerState = VictimsController ? Cast<AEverzonePlayerState>(VictimsController->PlayerState) : nullptr;
	if (KillersPlayerState && KillersPlayerState != VictimsPlayerState)
	{
		KillersPlayerState->AddToPlayerScore(1.0f);
	}
	if (VictimsPlayerState && KillersPlayerState)
	{
		VictimsPlayerState->AddToPlayerDeaths(1);
		FString KillersName = KillersPlayerState->GetPlayerName();
		VictimsPlayerState->SetKillersName(KillersName);
	}
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
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); // We minus one to ensure that the maximum value doesn't fall outside of the array index
		VictimsController->HideDeathMessage();
		RestartPlayerAtPlayerStart(VictimsController, PlayerStarts[Selection]);
	}
}


