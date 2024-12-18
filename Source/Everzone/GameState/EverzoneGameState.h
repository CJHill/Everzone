// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "EverzoneGameState.generated.h"

class AEverzonePlayerState; //forward declaration
/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzoneGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScorer(AEverzonePlayerState* TopPlayer);
	UPROPERTY(Replicated)
	TArray<AEverzonePlayerState*> TopScoringPlayers;

	//Team related properties

	TArray<AEverzonePlayerState*> OrangeTeam;
	TArray<AEverzonePlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_OrangeTeamScore)
	float OrangeTeamScore = 0;
	UFUNCTION()
	void OnRep_OrangeTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0;
	UFUNCTION()
	void OnRep_BlueTeamScore();
private:
	float TopScore = 0.f;
};
