// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EverzoneGameMode.generated.h"

namespace MatchState
{
    extern EVERZONE_API const FName CooldownState; // Match has concluded display winner starts cooldown timer
}
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
    UPROPERTY(EditDefaultsOnly)
    float MatchTime = 120.f;
    float LevelStartTime = 0.f;
    UPROPERTY(EditDefaultsOnly)
    float CooldownTime = 10.f;
protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;
private:
    float CountdownTime = 0.f;
public:
    FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
