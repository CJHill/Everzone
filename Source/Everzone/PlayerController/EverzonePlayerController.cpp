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
#include "Everzone/GameState/EverzoneGameState.h"
#include "Everzone/PlayerState/EverzonePlayerState.h"
#include "Components/Image.h"
#include "Everzone/HUD/ReturnToMenuWidget.h"
#include "Everzone/EverzoneTypes/Announcement.h"

void AEverzonePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	EverzoneHUD = Cast<AEverzoneHUD>(GetHUD());
	
	CheckMatchState();
}
void AEverzonePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;
	InputComponent->BindAction("Quit", EInputEvent::IE_Pressed, this, &AEverzonePlayerController::ShowReturnToMenu);
}
void AEverzonePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetHUDTime();
	PollInit();
	RefreshTimeSync(DeltaTime);
	CheckPing(DeltaTime);
	
	
}
void AEverzonePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEverzonePlayerController, StateofMatch);
	DOREPLIFETIME(AEverzonePlayerController, bShowTeamScores);
	
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
		LevelStartTime = EverzoneGameMode->LevelStartTime;//controller is created before the gamemode so we need to retrieve the time variables from the game mode class
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

void AEverzonePlayerController::PollInit() // purpose is to refresh these variables so that they always display the correct values
{
	if (CharacterOverlay == nullptr)
	{
		if (!EverzoneHUD || !EverzoneHUD->CharacterOverlay) return;
		CharacterOverlay = EverzoneHUD->CharacterOverlay;
		if (!CharacterOverlay) return;
		
		if(bInitHUDHealth) SetHUDHealth(HUDCurrentHealth, HUDMaxHealth);
		if(bInitHUDShield) SetHUDShield(HUDShield, HUDMaxShield);
		if(bInitHUDScore)  SetHUDScore(HUDScore);
		if(bInitHUDDeath)  SetHUDDeaths(HUDDeaths);
		if(bInitHUDAmmoReserves) SetHUDAmmoReserves(HUDAmmoReserves);
		if (bInitHUDWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
		if (bInitHUDWeaponIcon) ShowWeaponIcon(HUDWeaponIcon);

		EverzoneCharacter = Cast<AEverzoneCharacter>(GetPawn());
		if (EverzoneCharacter && EverzoneCharacter->GetCombatComp())
		{
			SetHUDGrenades(EverzoneCharacter->GetCombatComp()->GetGrenades());
		}
	}
}

void AEverzonePlayerController::RefreshTimeSync(float DeltaTime) // responsible for calling the server time request 
{
	TimeSinceLastSync += DeltaTime;
	if (IsLocalController() && TimeSinceLastSync > TimeSyncFrequency)
	{
		RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSinceLastSync = 0.f;
	}
}

void AEverzonePlayerController::ShowHighPingWarning()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->WifiIcon && 
		EverzoneHUD->CharacterOverlay->HighPingAnimation;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->WifiIcon->SetOpacity(1.f);
		EverzoneHUD->CharacterOverlay->PlayAnimation(EverzoneHUD->CharacterOverlay->HighPingAnimation);
	}

}

void AEverzonePlayerController::HideHighPingWarning()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->WifiIcon &&
		EverzoneHUD->CharacterOverlay->HighPingAnimation;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->WifiIcon->SetOpacity(0.f);
		if (EverzoneHUD->CharacterOverlay->IsAnimationPlaying(EverzoneHUD->CharacterOverlay->HighPingAnimation))
		{
			EverzoneHUD->CharacterOverlay->StopAnimation(EverzoneHUD->CharacterOverlay->HighPingAnimation);
		}
		
	}

}

void AEverzonePlayerController::CheckPing(float DeltaTime)
{
	HighPingTimeShown += DeltaTime;
	if (HighPingTimeShown > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > HighPingThreshold) // Ping is compressed as it's divided by 4. We need to multiply by 4 to return it to uncompressed format
			{
				CurrentPing = PlayerState->GetPing() * 4;
				ShowHighPingWarning();
				ServerReportPingStatus(true);
				PingAnimationDisplayTime = 0.f;
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingTimeShown = 0.f;
	}
	bool bIsHighPingAnimationPlaying = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->HighPingAnimation &&
		EverzoneHUD->CharacterOverlay->IsAnimationPlaying(EverzoneHUD->CharacterOverlay->HighPingAnimation);

	if (bIsHighPingAnimationPlaying)
	{
		PingAnimationDisplayTime += DeltaTime;
		if (PingAnimationDisplayTime > HighPingDuration)
		{
			HideHighPingWarning();
		}
	}
}
void AEverzonePlayerController::ShowReturnToMenu()
{
	if (ReturnToMenuWidget == nullptr) return;
	if (ReturnToMenu == nullptr)
	{
		ReturnToMenu = CreateWidget<UReturnToMenuWidget>(this, ReturnToMenuWidget);
	}
	if (ReturnToMenu)
	{
		bReturnToMenu = !bReturnToMenu;
		if (bReturnToMenu)
		{
			ReturnToMenu->MenuSetup();
		}
		else
		{
			ReturnToMenu->MenuTearDown();
		}
	}
	
}


//Is the ping too high?
void AEverzonePlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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
	SingleTripTime = 0.5 * RoundTripTime; // We are only making note of the single trip time so that it can be used for server side rewind
	float CurrentServerTime = TimeReceivedServerRequest + SingleTripTime;
	ClientServerDeltaTime = CurrentServerTime - GetWorld()->TimeSeconds;
}
void AEverzonePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	AEverzoneCharacter* PlayerCharacter = Cast<AEverzoneCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
		SetHUDShield(PlayerCharacter->GetShield(), PlayerCharacter->GetMaxShield());
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
		bInitHUDHealth = true;
		HUDCurrentHealth = CurrentHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AEverzonePlayerController::SetHUDShield(float ShieldAmount, float MaxShieldAmount)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->ShieldBar
		&& EverzoneHUD->CharacterOverlay->ShieldText;
	if (bIsHUDValid)
	{
		const float ShieldPercent = ShieldAmount / MaxShieldAmount;
		EverzoneHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(ShieldAmount), FMath::CeilToInt(MaxShieldAmount));
		EverzoneHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitHUDShield = true;
		HUDShield = ShieldAmount;
		HUDMaxShield = MaxShieldAmount;
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
		bInitHUDScore = true;
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
		bInitHUDDeath = true;
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
	else
	{
		bInitHUDWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
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
	else
	{
		bInitHUDAmmoReserves = true;
		HUDAmmoReserves = Ammo;
	}
}

void AEverzonePlayerController::SetHUDGrenades(int32 Grenades)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->GrenadesAmount;
	if (bIsHUDValid)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		EverzoneHUD->CharacterOverlay->GrenadesAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitHUDGrenade = true;
		HUDGrenades = Grenades;
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
		EverzoneHUD->CharacterOverlay->WeaponIcon->SetBrushFromTexture(WeaponIcon, false);
		EverzoneHUD->CharacterOverlay->WeaponIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		bInitHUDWeaponIcon = true;
		HUDWeaponIcon = WeaponIcon;
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
	if (!bIsHUDValid) return;
	
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
	if (TimeRemaining <= 30.f)
	{
		EverzoneHUD->CharacterOverlay->PlayAnimation(EverzoneHUD->CharacterOverlay->CountdownAnimation,0, 30);
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

void AEverzonePlayerController::HideTeamScores()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->OrangeTeamScore &&
		EverzoneHUD->CharacterOverlay->BlueTeamScore;
	if (bIsHUDValid)
	{
		EverzoneHUD->CharacterOverlay->OrangeTeamScore->SetText(FText());
		EverzoneHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
	}
}

void AEverzonePlayerController::InitTeamScores()
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->OrangeTeamScore &&
		EverzoneHUD->CharacterOverlay->BlueTeamScore;
	if (bIsHUDValid)
	{
		FString Zero("0");
		EverzoneHUD->CharacterOverlay->OrangeTeamScore->SetText(FText::FromString(Zero));
		EverzoneHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
	}
}

void AEverzonePlayerController::SetHUDOrangeTeamScores(int32 OrangeScores)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->OrangeTeamScore;
	if (bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), OrangeScores);
		EverzoneHUD->CharacterOverlay->OrangeTeamScore->SetText(FText::FromString(ScoreText));
	}

}

void AEverzonePlayerController::SetHUDBlueTeamScores(int32 BlueScores)
{
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	bool bIsHUDValid = EverzoneHUD &&
		EverzoneHUD->CharacterOverlay &&
		EverzoneHUD->CharacterOverlay->BlueTeamScore;
	if (bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScores);
		EverzoneHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}
void AEverzonePlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
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

void AEverzonePlayerController::BroadcastElim(APlayerState* Killer, APlayerState* Victim)
{
	ClientElimAnnouncement(Killer, Victim);
}
void AEverzonePlayerController::ClientElimAnnouncement_Implementation(APlayerState* Killer, APlayerState* Victim)
{
	//Self represents the locally controlled player
	APlayerState* Self = GetPlayerState<APlayerState>();

	if (Killer, Victim, Self)
	{
		EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
		if (!EverzoneHUD) return;

		//You killed someone conditional
		if (Killer == Self && Victim != Self)
		{
			EverzoneHUD->AddElimAnnouncementOverlay("You", Victim->GetPlayerName());
			return;
		}

		//Someone killed you conditional
		if (Victim == Self && Killer != Self)
		{
			EverzoneHUD->AddElimAnnouncementOverlay(Killer->GetPlayerName(), "You");
			return;
		}

		//Someone else killed themselves conditional
		if (Killer == Victim && Killer != Self)
		{
			EverzoneHUD->AddElimAnnouncementOverlay(Killer->GetPlayerName(), "themselves");
			return;
		}

		//You killed yourself conditional
		if (Killer == Victim && Killer == Self)
		{
			EverzoneHUD->AddElimAnnouncementOverlay("You", "yourself");
			return;
		}
		
		//If we reach this line it means someone killed someone else
		EverzoneHUD->AddElimAnnouncementOverlay(Killer->GetPlayerName(), Victim->GetPlayerName());
	}
}

void AEverzonePlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	StateofMatch = State;
	
	if (StateofMatch == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
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

void AEverzonePlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	if (EverzoneHUD)
	{
		EverzoneHUD->AddCharacterOverlay();
		if (EverzoneHUD->AnnouncementOverlay)
		{
			EverzoneHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AEverzonePlayerController::HandleCooldown()
{
	AEverzoneGameState* EverzoneGameState = Cast<AEverzoneGameState>(UGameplayStatics::GetGameState(this));
	AEverzonePlayerState* EverzonePlayerState = GetPlayerState<AEverzonePlayerState>();
	EverzoneHUD = EverzoneHUD == nullptr ? EverzoneHUD = Cast<AEverzoneHUD>(GetHUD()) : EverzoneHUD;
	if (!EverzoneHUD) return;
	
	EverzoneHUD->CharacterOverlay->RemoveFromParent();
	bool bIsHUDValid = EverzoneHUD->AnnouncementOverlay &&
	EverzoneHUD->AnnouncementOverlay->Announcement &&
	EverzoneHUD->AnnouncementOverlay->InputInfo;

	if (!bIsHUDValid) return;
	FString AnnouncementText = Announcement::NewMatchStartsIn;
	EverzoneHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	EverzoneHUD->AnnouncementOverlay->Announcement->SetText(FText::FromString(AnnouncementText));

	if (EverzoneGameState && EverzonePlayerState)
	{
		TArray<AEverzonePlayerState*> TopPlayers = EverzoneGameState->TopScoringPlayers;
		FString InfoText = bShowTeamScores ? GetTeamsAnnouncementText(EverzoneGameState) : GetAnnouncementInfoText(TopPlayers);
		
			
		EverzoneHUD->AnnouncementOverlay->InputInfo->SetText(FText::FromString(InfoText));
	}
	
	EverzoneCharacter = Cast<AEverzoneCharacter>(GetPawn());
	
	if (EverzoneCharacter && EverzoneCharacter->GetCombatComp())
	{
		EverzoneCharacter->bDisableGameplay = true;
		EverzoneCharacter->GetCombatComp()->ShootButtonPressed(false);
	}
}

FString AEverzonePlayerController::GetAnnouncementInfoText(const TArray<AEverzonePlayerState*>& BestPlayers)
{
	FString AnnouncementInfoText;
	if (BestPlayers.Num() == 0)
	{
		AnnouncementInfoText = Announcement::ThereIsNoWinner;
	}
	else if (BestPlayers.Num() == 1)
	{
		AnnouncementInfoText = Announcement::OneWinner;
		AnnouncementInfoText.Append(FString::Printf(TEXT("\n%s"), *BestPlayers[0]->GetPlayerName()));
	}
	else if (BestPlayers.Num() > 1)
	{
		AnnouncementInfoText = Announcement::MultipleWinners;
		AnnouncementInfoText.Append(FString("\n"));
		for (auto JointFirstPlayers : BestPlayers)
		{
			AnnouncementInfoText.Append(FString::Printf(TEXT("%s\n"), *JointFirstPlayers->GetPlayerName()));
		}
	}
	return AnnouncementInfoText;
}

FString AEverzonePlayerController::GetTeamsAnnouncementText(AEverzoneGameState* GameState)
{
	if(GameState == nullptr) return FString();
	FString AnnouncementInfoText;

	const int32 OrangeTeamScore = GameState->OrangeTeamScore;
	const int32 BlueTeamScore = GameState->BlueTeamScore;

	if (OrangeTeamScore == 0 && BlueTeamScore == 0)
	{
		AnnouncementInfoText = Announcement::ThereIsNoWinner;
	}
	else if (OrangeTeamScore == BlueTeamScore)
	{
		AnnouncementInfoText = FString::Printf(TEXT("%s\n"), *Announcement::TeamTied);
		AnnouncementInfoText.Append(Announcement::OrangeTeam);
		AnnouncementInfoText.Append(FString("\n"));
		AnnouncementInfoText.Append(Announcement::BlueTeam);
		AnnouncementInfoText.Append(FString("\n"));
	}
	else if (OrangeTeamScore > BlueTeamScore)
	{
		AnnouncementInfoText = Announcement::OrangeTeamWins;
		AnnouncementInfoText.Append(FString("\n"));
		AnnouncementInfoText.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::OrangeTeam, OrangeTeamScore));
		AnnouncementInfoText.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > OrangeTeamScore)
	{
		AnnouncementInfoText = Announcement::BlueTeamWins;
		AnnouncementInfoText.Append(FString("\n"));
		AnnouncementInfoText.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		AnnouncementInfoText.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::OrangeTeam, OrangeTeamScore));
	}
	return AnnouncementInfoText;
}
