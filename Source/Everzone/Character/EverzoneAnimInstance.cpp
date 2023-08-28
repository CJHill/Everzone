// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneAnimInstance.h"
#include "EverzoneCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEverzoneAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	EverzoneCharacter = Cast<AEverzoneCharacter>(TryGetPawnOwner());
}

void UEverzoneAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (EverzoneCharacter == nullptr)
	{
		EverzoneCharacter = Cast<AEverzoneCharacter>(TryGetPawnOwner());
	}
	if (EverzoneCharacter == nullptr) return;

	FVector Velocity = EverzoneCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	bIsInAir = EverzoneCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = EverzoneCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bIsWeaponEquipped = EverzoneCharacter->IsWeaponEquipped();
	bIsCrouched = EverzoneCharacter->bIsCrouched;
}
