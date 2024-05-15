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
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DeathAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DeathMessage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KilledBy;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoReserves;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchTimer;

	UPROPERTY(meta = (BindWidget))
	class UImage* WeaponIcon;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* CountdownAnimation;
};
