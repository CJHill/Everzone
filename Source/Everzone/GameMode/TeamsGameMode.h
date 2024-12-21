// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EverzoneGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API ATeamsGameMode : public AEverzoneGameMode
{
	GENERATED_BODY()
	
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual float CalculateDamage(AController* Killer, AController* Victim, float BaseDamage) override;

	virtual void PlayerEliminated(class AEverzoneCharacter* PlayerKilled, class AEverzonePlayerController* VictimsController, class AEverzonePlayerController* KillersController) override;
protected:
	virtual void HandleMatchHasStarted() override;
};
