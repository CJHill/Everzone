// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneHUD.h"
#include "GameFramework/PlayerController.h"
#include "OverlayWidget.h"
#include "AnnouncementWidget.h"
#include "ElimAnnouncementWidget.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void AEverzoneHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEverzoneHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && OverlayWidgetClass)
	{
		CharacterOverlay = CreateWidget<UOverlayWidget>(PlayerController, OverlayWidgetClass);
		CharacterOverlay->AddToViewport();
	}
}
void AEverzoneHUD::AddAnnouncementOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementWidgetClass)
	{
		if (AnnouncementOverlay) return;
		AnnouncementOverlay = CreateWidget<UAnnouncementWidget>(PlayerController, AnnouncementWidgetClass);
		AnnouncementOverlay->AddToViewport();
	}
}
void AEverzoneHUD::AddElimAnnouncementOverlay(FString Killer, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncementWidget* ElimAnnouncementWidget = CreateWidget<UElimAnnouncementWidget>(OwningPlayer, ElimAnnouncementClass);
		if (!ElimAnnouncementWidget) return;

		ElimAnnouncementWidget->SetElimAnnouncementText(Killer, Victim);
		ElimAnnouncementWidget->AddToViewport();

		for (auto Message : ElimMessages)
		{
			if (!Message || !Message->AnnouncementBox) return;

			UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
			if (!CanvasSlot) return;

			FVector2D MessagePosition = CanvasSlot->GetPosition();
			FVector2D NewMessagePosition(CanvasSlot->GetPosition().X, MessagePosition.Y - CanvasSlot->GetSize().Y);

			CanvasSlot->SetPosition(NewMessagePosition);
		}

		ElimMessages.Add(ElimAnnouncementWidget);

		FTimerHandle ElimAnnouncementTimer;
		FTimerDelegate ElimAnnouncementDelegate;

		ElimAnnouncementDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		GetWorld()->GetTimerManager().SetTimer(ElimAnnouncementTimer, ElimAnnouncementDelegate, ElimAnnouncementTime, false);
	}
}
void AEverzoneHUD::ElimAnnouncementTimerFinished(UElimAnnouncementWidget* AnnouncementToRemove)
{
	if (AnnouncementToRemove)
	{
		ElimMessages.Remove(AnnouncementToRemove);
		AnnouncementToRemove->RemoveFromParent();
		
	}
}
void AEverzoneHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D Viewport;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(Viewport);
		const FVector2D ViewportCenter(Viewport.X / 2.f, Viewport.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColour);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter,Spread, HUDPackage.CrosshairColour);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter,Spread, HUDPackage.CrosshairColour);
		}
		if (HUDPackage.CrosshairUp)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairUp, ViewportCenter, Spread, HUDPackage.CrosshairColour);
		}
		if (HUDPackage.CrosshairDown)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairDown, ViewportCenter, Spread, HUDPackage.CrosshairColour);
		}
	}
}


/*
* DrawCrosshair: This is where the size of the crosshairs can be customised
  draw texture  is responsible for drawing the crosshairs to the HUD 
*/
void AEverzoneHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColour)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, CrosshairsColour);
}


