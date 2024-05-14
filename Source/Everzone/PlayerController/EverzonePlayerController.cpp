// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzonePlayerController.h"
#include "Everzone/HUD/EverzoneHUD.h"
#include "Everzone/HUD/OverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "Everzone/GameMode/EverzoneGameMode.h"
#include "Everzone/HUD/AnnouncementWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Everzone/EverzoneComponents/CombatComponent.h"

void AEverzonePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	EverzoneHUD = Cast<AEverzoneHUD>(GetHUD());
	
	CheckMatchState();
}
void AEverzonePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	PollInit();
	RefreshTimeSync(DeltaTime);
	
}
void AEverzonePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEverzonePlayerController, StateofMatch);
	
}
void AEverzonePlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (HasAuthority())
	{		
		EverzoneGameMode = EverzoneGameMode== nullptr ? Cast<AEverzoneGameMode>(UGameplayStatics::GetGameMode(this)) : EverzoneGameMode;
	   if (EverzoneGameMode)
	   {
		LevelStartTime = EverzoneGameMode->LevelStartTime;
		WarmUpTime = EverzoneGameMode->WarmUpTime;
		MatchTime = EverzoneGameMode->MatchTime;
		SecondsLeft = FMath::CeilToInt(EverzoneGameMode->GetCountdownTime() + LevelStartTime);
	   }
    }
	
	if (StateofMatch == MatchState::WaitingToStart)TimeLeft = WarmUpTime - GetCurrentServerTime() + LevelStartTime;
	else if (StateofMatch == MatchState::InProgress) TimeLeft = WarmUpTime + MatchTime - GetCurrentServerTime() + LevelStartTime;
	else if (StateofMatch == MatchState::CooldownState) TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetCurrentServerTime() + LevelStartTime;
	
	uint32 GameTimeLeft = FMath::CeilToInt(TimeLeft);
	if (MatchTimerInt != GameTimeLeft)
	{
		if (StateofMatch == MatchState::WaitingToStart || StateofMatch == MatchState::CooldownState)
		{
		
			SetHUDAnnouncementTimer(TimeLeft);

		}
		if (StateofMatch == MatchState::InProgress)
		{
			SetHUDMatchTimer(TimeLeft);
		}
	}
	MatchTimerInt = GameTimeLeft;
	
	
}

void AEverzonePlayerController::PollInit()
{
	if (CharacterOverlay != nullptr) return;
	
	if (EverzoneHUD && EverzoneHUD->CharacterOverlay)
	{
		CharacterOverlay = EverzoneHUD->CharacterOverlay;
		if (CharacterOverlay)
		{
			SetHUDHealth(HUDCurrentHealth, HUDMaxHealth);
			SetHUDScore(HUDScore);
			SetHUDDeaths(HUDDeaths);
		}
	}
	
}

void AEverzonePlayerController::RefreshTimeSync(float DeltaTime)
{
	TimeSinceLastSync += DeltaTime;
	if (IsLocalController() && TimeSinceLastSync > TimeSyncFrequency)
	{
		RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSinceLastSync = 0.f;
	}
}

void AEverzonePlayerController::CheckMatchState_Implementation()
{
	AEverzoneGameMode* GameMode = Cast<AEverzoneGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartTime = GameMode->LevelStartTime;
		CooldownTime = GameMode->CooldownTime;
		StateofMatch = GameMode->GetMatchState();
		ClientJoinMidGame(StateofMatch, WarmUpTime,CooldownTime,MatchTime, LevelStartTime);
	}
	
}
void AEverzonePlayerController::ClientJoinMidGame_Implementation(FName StateOfTheMatch, float Warmup,float Cooldown, float Match, float StartingTime)
{
	WarmUpTime = Warmup;
	StateofMatch = StateOfTheMatch;
	LevelStartTime = StartingTime;
	CooldownTime = Cooldown;
	MatchTime = Match;
	OnMatchStateSet(StateofMatch);
	if (EverzoneHUD && StateofMatch == MatchState::WaitingToStart)
	{
		EverzoneHUD->AddAnnouncementOverlay();

	}

}
void AEverzonePlayerController::RequestServerTime_Implementation(float TimeServerRequest)
{
	float ServerTimeofRequestReceived = GetWorld()->TimeSeconds;
	ReportServerTime(TimeServerRequest, ServerTimeofRequestReceived);

}
void AEverzonePlayerController::ReportServerTime_Implementation(float TimeOfServerRequest, float TimeReceivedServerRequest)
{
	float RoundTripTime = GetWorld()->TimeSeconds - TimeOfServerRequest;
	float CurrentServerTime = TimeReceivedServerRequest + (0.5 * RoundTripTime);
	ClientServerDeltaTime = CurrentServerTime - GetWorld()->TimeSeconds;
}
void AEverzonePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AEverzoneCharacter* PlayerCharacter = Cast<AEverzoneCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
		HideDeathMessage();
	}
}

void AEverzonePlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD && 
		EverzoneHUD->CharacterOverlay && 
		EverzoneHUD->CharacterOverlay->HealthBar 
		&& EverzoneHUD->CharacterOverlay->HealthText;
	if (bIsHUDValid)
	{
		const float HealthPercent = CurrentHealth / MaxHealth;
		EverzoneHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
		EverzoneHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitCharacterOverlay = true;
		HUDCurrentHealth = CurrentHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AEverzonePlayerController::SetHUDScore(float Score)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->ScoreAmount;
	if (bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		EverzoneHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else 
	{
		bInitCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AEverzonePlayerController::SetHUDDeaths(int32 Deaths)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->DeathAmount;
	if (bIsHUDValid)
	{
		FString DeathText = FString::Printf(TEXT("%d"), Deaths);
		EverzoneHUD->CharacterOverlay->DeathAmount->SetText(FText::FromString(DeathText));
	}
	else
	{
		bInitCharacterOverlay = true;
		HUDDeaths = Deaths;
	}
}

void AEverzonePlayerController::ShowDeathMessage(const FString KilledBy)
{
	if (KilledBy == FString(""))
	{
		return;
	}
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->DeathMessage &&
		EverzoneHUD->CharacterOverlay->KilledBy;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->KilledBy->SetText(FText::FromString(KilledBy));
		EverzoneHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		EverzoneHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		
	}
}

void AEverzonePlayerController::HideDeathMessage()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->DeathMessage && 
		EverzoneHUD->CharacterOverlay->KilledBy;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Collapsed);
		EverzoneHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AEverzonePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->AmmoAmount;
	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		EverzoneHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AEverzonePlayerController::SetHUDAmmoReserves(int32 Ammo)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->AmmoReserves;
	if (bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		EverzoneHUD->CharacterOverlay->AmmoReserves->SetText(FText::FromString(AmmoText));
	}
}

void AEverzonePlayerController::ShowWeaponIcon(UTexture2D* WeaponIcon)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->WeaponIcon;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->WeaponIcon->SetBrushFromTexture(WeaponIcon, true);
		EverzoneHUD->CharacterOverlay->WeaponIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AEverzonePlayerController::HideWeaponIcon()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->WeaponIcon;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->WeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AEverzonePlayerController::SetHUDMatchTimer(float TimeRemaining)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->MatchTimer;
	if (bIsHUDValid)
	{
		if (TimeRemaining < 0.f)
		{
			// Safety check see announcment timer function for the reason.
			EverzoneHUD->CharacterOverlay->MatchTimer->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.f);
		int32 Seconds = TimeRemaining - Minutes * 60.f;
		//"%02d": the 2 represents the amount of digits you want the int variable to display, 
		//the 0 respresents the padding by setting it 0 it will format the first digit as a zero if the minutes or seconds are a single digit. 
		//changing the 0 will turn the padding into spaces from the left
		FString MatchTimerText = FString::Printf(TEXT(" %02d:%02d"), Minutes, Seconds); 
		EverzoneHUD->CharacterOverlay->MatchTimer->SetText(FText::FromString(MatchTimerText));
	}
}

void AEverzonePlayerController::SetHUDAnnouncementTimer(float TimeRemaining)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
	EverzoneHUD->AnnouncementOverlay &&
	EverzoneHUD->AnnouncementOverlay->WarmUpTimer;
	if (bIsHUDValid)
	{
		if (TimeRemaining < 0.f)
		{
			// the time remaining initally starts at -1 on client machines this will hide the text until the client has obtained the correct time values.
			EverzoneHUD->AnnouncementOverlay->WarmUpTimer->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.f);
		int32 Seconds = TimeRemaining - Minutes * 60.f;
		FString WarmUpTimerText = FString::Printf(TEXT(" %02d:%02d"), Minutes, Seconds);
		EverzoneHUD->AnnouncementOverlay->WarmUpTimer->SetText(FText::FromString(WarmUpTimerText));
	}
}

float AEverzonePlayerController::GetCurrentServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime;
	}
	
}

void AEverzonePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AEverzonePlayerController::OnMatchStateSet(FName State)
{
	StateofMatch = State;
	
	if (StateofMatch == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (StateofMatch == MatchState::CooldownState)
	{
		HandleCooldown();
	}
}

void AEverzonePlayerController::OnRep_MatchState()
{
	if (StateofMatch == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (StateofMatch == MatchState::CooldownState)
	{
		HandleCooldown();
	}
}


void AEverzonePlayerController::HandleMatchHasStarted()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	if (EverzoneHUD)
	{
		EverzoneHUD->AddCharacterOverlay();
		if (EverzoneHUD->AnnouncementOverlay)
		{
			EverzoneHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AEverzonePlayerController::HandleCooldown()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	if (EverzoneHUD)
	{
		EverzoneHUD->CharacterOverlay->RemoveFromParent();
		bool bIsHUDValid = EverzoneHUD->AnnouncementOverlay &&
			EverzoneHUD->AnnouncementOverlay->Announcement &&
			EverzoneHUD->AnnouncementOverlay->InputInfo;
		if (bIsHUDValid)
		{
			FString AnnouncementText("New Match Starts In: ");
			EverzoneHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			EverzoneHUD->AnnouncementOverlay->Announcement->SetText(FText::FromString(AnnouncementText));
			EverzoneHUD->AnnouncementOverlay->InputInfo->SetText(FText());
		}
	}
	EverzoneCharacter = Cast<AEverzoneCharacter>(GetPawn());
	
	if (EverzoneCharacter && EverzoneCharacter->GetCombatComp())
	{
		EverzoneCharacter->bDisableGameplay = true;
		EverzoneCharacter->GetCombatComp()->ShootButtonPressed(false);
	}
}

