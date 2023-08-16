// Fill out your copyright notice in the Description page of Project Settings.


#include "EverzoneCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEverzoneCharacter::AEverzoneCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}


void AEverzoneCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
void AEverzoneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &AEverzoneCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AEverzoneCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AEverzoneCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AEverzoneCharacter::LookUp);

}
void AEverzoneCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		//creating a rotation matrix by passing in a FRotator which then gets and returns the X axis
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, value);
	}
}

void AEverzoneCharacter::MoveRight(float value)
{
	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	AddMovementInput(Direction, value);
}

void AEverzoneCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void AEverzoneCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}


void AEverzoneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



