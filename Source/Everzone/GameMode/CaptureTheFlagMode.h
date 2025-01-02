// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagMode.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API ACaptureTheFlagMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AEverzoneCharacter* PlayerKilled, class AEverzonePlayerController* VictimsController, class AEverzonePlayerController* KillersController) override;
	void FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone);

	//Edit this variable to set how many points a flag capture earns you
	UPROPERTY(EditAnywhere)
	float FlagCaptureScore = 30;
};
