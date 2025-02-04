// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementWidget.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API UAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Announcement;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InputInfo;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmUpTimer;

	
};
