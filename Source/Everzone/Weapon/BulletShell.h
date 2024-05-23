// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS() 
class EVERZONE_API ABulletShell : public AActor
{
	GENERATED_BODY()
	//This class is for any weapon that ejects bullet shells. These bullet shells are purely for cosmetic purposes
public:	
	// Sets default values for this actor's properties
	ABulletShell();

	virtual void Tick(float DeltaTime) override;
protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShellMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
public:	
	
	

};
