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

private:
	float TopScore = 0.f;
};
