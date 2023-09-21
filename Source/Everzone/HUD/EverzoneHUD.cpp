// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneHUD.h"

void AEverzoneHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D Viewport;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(Viewport);
		const FVector2D ViewportCenter(Viewport.X / 2.f, Viewport.Y / 2.f);

		if (HUDPackage.CrosshairCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter);
		}
		if (HUDPackage.CrosshairLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter);
		}
		if (HUDPackage.CrosshairRight)
		{
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter);
		}
		if (HUDPackage.CrosshairUp)
		{
			DrawCrosshair(HUDPackage.CrosshairUp, ViewportCenter);
		}
		if (HUDPackage.CrosshairDown)
		{
			DrawCrosshair(HUDPackage.CrosshairDown, ViewportCenter);
		}
	}
}

void AEverzoneHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f), ViewportCenter.Y - (TextureHeight / 2.f));
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}
