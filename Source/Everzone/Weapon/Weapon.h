// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class EVERZONE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
protected:
	
	virtual void BeginPlay() override;
private:
	USkeletalMesh* WeaponMesh;
public:	
	
	

};
