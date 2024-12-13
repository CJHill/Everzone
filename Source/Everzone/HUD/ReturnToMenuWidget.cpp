// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Everzone/Character/EverzoneCharacter.h"

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

	if (ReturnBtn && !ReturnBtn->OnClicked.IsBound())
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

	PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
	if (PlayerController)
	{
		FInputModeGameOnly InputModeData;

		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
	if (ReturnBtn && ReturnBtn->OnClicked.IsBound())
	{
		ReturnBtn->OnClicked.RemoveDynamic(this, &UReturnToMenuWidget::ReturnButtonClicked);
	}
	if (MultiplayerSessions)
	{
		MultiplayerSessions->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMenuWidget::OnDestroyedSession);
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
			UE_LOG(LogTemp, Warning, TEXT(" GameMode returning to main menu"));
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

void UReturnToMenuWidget::OnPlayerLeftGame()
{
	if (MultiplayerSessions)
	{
		MultiplayerSessions->DestroySession();
	}
}

void UReturnToMenuWidget::ReturnButtonClicked()
{
	ReturnBtn->SetIsEnabled(false);
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* FirstController = World->GetFirstPlayerController();
	if (!FirstController) return;

	AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(FirstController->GetPawn());
	if (EverzoneCharacter)
	{
		EverzoneCharacter->ServerLeftGame();
		EverzoneCharacter->OnLeftGame.AddDynamic(this, &UReturnToMenuWidget::OnPlayerLeftGame);
	}
	else
	{
		ReturnBtn->SetIsEnabled(true);
	}
}
