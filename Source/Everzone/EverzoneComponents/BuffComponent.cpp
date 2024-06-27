// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UBuffComponent::HealOverTime(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	// Getting the players health and adding the amount to heal during a single frame. Clamping this calculation between 0 and the max health of the player
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::Heal(float HealAmount, float HealOverTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealOverTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::BuffSpeed(float BaseSpdIncrease, float BaseCrouchSpdIncrease, float SpdBuffTime)
{
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(SpeedTimer, this, &UBuffComponent::ResetSpeedTimer, SpdBuffTime);
	}
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpdIncrease;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpdIncrease;
	}
	MulticastSpdBuff(BaseSpdIncrease, BaseCrouchSpdIncrease);
}

void UBuffComponent::SetInitialSpd(float BaseSpd, float BaseCrouchSpd)
{
	InitialBaseSpd = BaseSpd;
	InitialCrouchSpd = BaseCrouchSpd;
}
void UBuffComponent::ResetSpeedTimer()
{
	if (!Character || !Character->GetCharacterMovement()) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpd;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpd;
	MulticastSpdBuff(InitialBaseSpd, InitialCrouchSpd);
}

void UBuffComponent::MulticastSpdBuff_Implementation(float BaseSpd, float BaseCrouchSpd)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpd;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpd;
}

// Called every frame
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealOverTime(DeltaTime);
}

