// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncementWidget.h"
#include "Components/TextBlock.h"

void UElimAnnouncementWidget::SetElimAnnouncementText(FString KillerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s killed %s!"), *KillerName, *VictimName);

	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
