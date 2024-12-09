// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API UReturnToMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void MenuSetup();
	void MenuTearDown();
protected:
	virtual bool Initialize() override;
	void OnDestroyedSession(bool bWasDestroyed);
private:
	UPROPERTY(meta =(BindWidget))
	class UButton* ReturnBtn;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessions;

	UPROPERTY()
	class APlayerController* PlayerController;
};
