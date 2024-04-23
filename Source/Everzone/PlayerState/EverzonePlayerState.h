// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EverzonePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzonePlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Replication Notifications
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Deaths();

	void AddToPlayerScore(float ScoreAmount);
	void AddToPlayerDeaths(int32 DeathAmount);
private:
	//Note: It's important to make sure pointers are giving UProperty Macro to make sure they are null without doing so results in undefined behaviour
	UPROPERTY()
	class AEverzoneCharacter* PlayerCharacter;

	UPROPERTY()
	class AEverzonePlayerController* PlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;

	
};
