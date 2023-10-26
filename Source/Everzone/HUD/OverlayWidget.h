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
	/*These variables are responible for changing the variables of the same name in the CharacterOverlay Blueprint*/
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
};
