// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverlayWidget.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API UOverlayWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/*These variables are responsible for changing the variables of the same name in the CharacterOverlay  Widget Blueprint*/
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
    UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OrangeTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathMessage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KilledBy;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoReserves;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchTimer;

	UPROPERTY(meta = (BindWidget))
	class UImage* WeaponIcon;
	UPROPERTY(meta = (BindWidget))
	class UImage* WifiIcon;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* HighPingAnimation;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* CountdownAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesAmount;
};
