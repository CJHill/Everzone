// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FHitBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FHitBoxInfo> HitboxMap;

	UPROPERTY()
	AEverzoneCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;
	UPROPERTY()
	bool bHitHeadshot;
};
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AEverzoneCharacter*, uint32> Headshots;
	UPROPERTY()
	TMap<AEverzoneCharacter*, uint32> Bodyshots;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EVERZONE_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();
	friend class AEverzoneCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Colour);

	FServerSideRewindResult HitScanServerSideRewind(class AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	FServerSideRewindResult ProjectileServerSideRewind(class AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AEverzoneCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	//ServerScore Request is for Hitscan Weapons
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	UFUNCTION(Server, Reliable)
	void ServerProjectileScoreRequest(class AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<AEverzoneCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& PackageToSave);

	FFramePackage FrameToInterp(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	
	void CacheHitBoxPositions(AEverzoneCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveHitBoxes(AEverzoneCharacter* HitCharacter, const FFramePackage& FramePackage);
	void ResetHitBoxes(AEverzoneCharacter* HitCharacter, const FFramePackage& FramePackage);

	void EnableCharactersMeshCollision(AEverzoneCharacter* HitCharacter, ECollisionEnabled::Type CollsionEnabled);

	void SaveFrame();
    FFramePackage GetFrameToCheck(AEverzoneCharacter* HitCharacter, float HitTime);

	//Confirm Hit functions. Confirm hit is for hitscan weapons
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);
private:
	UPROPERTY()
	AEverzoneCharacter* Character;
	UPROPERTY()
	class AEverzonePlayerController* PlayerController;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 0.3f;
public:	
	// Called every frame


		
};
