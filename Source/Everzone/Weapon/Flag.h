// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class EVERZONE_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	//We are overriding dropped to detach the flag mesh as its a static mesh whereas the other weapons are skeletal
	virtual void Dropped() override;
protected:

	virtual void HandleOnEquipped() override;
	virtual void HandleOnDropped() override;
private:

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* FlagMesh;
};
