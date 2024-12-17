// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EverzoneHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairUp;
	UTexture2D* CrosshairDown;
	float CrosshairSpread;
	FLinearColor CrosshairColour;
};
/**
 * 
 */
UCLASS()
class EVERZONE_API AEverzoneHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
    void AddCharacterOverlay();
	void AddAnnouncementOverlay();
	void AddElimAnnouncementOverlay(FString Killer, FString Victim);

	UPROPERTY(EditAnywhere, Category = "Player Properties")
	TSubclassOf<class UUserWidget> OverlayWidgetClass;
	UPROPERTY()
	class UOverlayWidget* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> AnnouncementWidgetClass;

	UPROPERTY()
	class UAnnouncementWidget* AnnouncementOverlay;
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColour);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncementWidget> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 1.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncementWidget* AnnouncementToRemove);

	UPROPERTY()
	TArray<UElimAnnouncementWidget*> ElimMessages;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
