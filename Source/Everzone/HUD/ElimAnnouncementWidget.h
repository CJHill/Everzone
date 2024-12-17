// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncementWidget.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API UElimAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetElimAnnouncementText(FString KillerName, FString VictimName);

	//Variables found in the editor blueprint
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox; 

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;
};
