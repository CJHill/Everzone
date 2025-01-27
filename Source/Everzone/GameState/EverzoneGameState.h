// This class is responsible for handling the game's score not only for score in Team gamemodes but also for free for all as well.
// It also holds the Teams arrays.

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

	void OrangeTeamScores();
	void BlueTeamScores();
	void OrangeTeamScores(float Score);
	void BlueTeamScores(float Score);

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
