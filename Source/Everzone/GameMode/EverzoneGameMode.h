// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EverzoneGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzoneGameMode : public AGameMode
{
	GENERATED_BODY()

public:
virtual void PlayerEliminated(class AEverzoneCharacter* PlayerKilled, class AEverzonePlayerController* VictimsController, class AEverzonePlayerController* KillersController);
virtual void RequestRespawn(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController);
};
