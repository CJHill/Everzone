// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneAnimInstance.h"
#include "EverzoneCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Everzone/Weapon/Weapon.h"

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
	EquippedWeapon = EverzoneCharacter->GetEquippedWeapon();
	bIsCrouched = EverzoneCharacter->bIsCrouched;
	bIsAiming = EverzoneCharacter->IsAiming();
	TurningInPlace = EverzoneCharacter->GetTurningInPlace();
	bRotateRootBone = EverzoneCharacter->RotateRootBone();
	bIsEliminated = EverzoneCharacter->IsEliminated();

	//These variables and normalised delta rotator are needed for character strafing
	FRotator AimRotation = EverzoneCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(EverzoneCharacter->GetVelocity());
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = EverzoneCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = EverzoneCharacter->GetAO_Yaw();
	AO_Pitch = EverzoneCharacter->GetAO_Pitch();

	if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && EverzoneCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		EverzoneCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (EverzoneCharacter->IsLocallyControlled())
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = EverzoneCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(FVector3d(), RightHandTransform.GetLocation() - EverzoneCharacter->GetHitTarget());
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
		
		
	}
	
}

