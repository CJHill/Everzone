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
	AEverzoneGameMode();
    virtual void Tick(float DeltaTime) override;
    virtual void PlayerEliminated(class AEverzoneCharacter* PlayerKilled, class AEverzonePlayerController* VictimsController, class AEverzonePlayerController* KillersController);
    virtual void RequestRespawn(AEverzoneCharacter* PlayerKilled, AEverzonePlayerController* VictimsController);


    UPROPERTY(EditDefaultsOnly)
    float WarmUpTime = 10.f;

    float LevelStartTime = 0.f;
protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;
private:
    float CountdownTime = 0.f;

};
