// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMenuWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (!World) return;

	PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
	if (PlayerController)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(true);
	}

	if (ReturnBtn)
	{
		ReturnBtn->OnClicked.AddDynamic(this, &UReturnToMenuWidget::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	MultiplayerSessions = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (MultiplayerSessions)
	{
		MultiplayerSessions->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMenuWidget::OnDestroyedSession);
	}
}

void UReturnToMenuWidget::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (!World) return;

    PlayerController = PlayerController ==nullptr ? World->GetFirstPlayerController() : PlayerController;
	if (PlayerController)
	{
		FInputModeGameOnly InputModeData;
		
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
}

bool UReturnToMenuWidget::Initialize()
{
	if(!Super::Initialize()) return false;

	return true;
	
}

void UReturnToMenuWidget::OnDestroyedSession(bool bWasDestroyed)
{
	if (!bWasDestroyed)
	{
		ReturnBtn->SetIsEnabled(true);
		return;
	}
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMenuWidget::ReturnButtonClicked()
{
	ReturnBtn->SetIsEnabled(false);
	if (MultiplayerSessions)
	{
		MultiplayerSessions->DestroySession();
	}
}
