// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Everzone/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

	
}




// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
	
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& PackageToSave)
{
	Character = Character == nullptr ? Cast<AEverzoneCharacter>(GetOwner()) : Character;
	if (Character)
	{
		PackageToSave.Time = GetWorld()->GetTimeSeconds();
		PackageToSave.Character = Character;
		for (auto& BoxPair : Character->PlayerHitBoxes)
		{
			FHitBoxInfo BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			PackageToSave.HitboxMap.Add(BoxPair.Key, BoxInfo);
		}
	}
}
FFramePackage ULagCompensationComponent::FrameToInterp(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0,1);

	FFramePackage InterpFPackage;
	InterpFPackage.Time = HitTime;
	for (auto& YoungerPair : YoungerFrame.HitboxMap )
	{
		const FName& HitBoxInfoName = YoungerPair.Key;

		const FHitBoxInfo& OlderBox = OlderFrame.HitboxMap[HitBoxInfoName];
		const FHitBoxInfo& YoungerBox = YoungerFrame.HitboxMap[HitBoxInfoName];

		FHitBoxInfo InterpHitBoxInfo;
		InterpHitBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpHitBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpHitBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFPackage.HitboxMap.Add(HitBoxInfoName, InterpHitBoxInfo);
	}
	return InterpFPackage;
}
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	//Storing the position of Hit character's hitboxes in its current position 
	CacheHitBoxPositions(HitCharacter, CurrentFrame);
	//Moving the hit characters position back to the frame where the hit took place
	MoveHitBoxes(HitCharacter, Package);
	EnableCharactersMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//Enable Collision for the head hitbox.
	UBoxComponent* HeadBox = HitCharacter->PlayerHitBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult ConfirmedHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (!World) return FServerSideRewindResult();
	
	World->LineTraceSingleByChannel(ConfirmedHitResult, TraceStart, TraceEnd, ECC_Visibility);
	if (ConfirmedHitResult.bBlockingHit) // If we hit the head reset hit boxes to the original state via the cached frame.
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharactersMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true,true };
	}
	else // Didn't the head check the rest of the body
	{
		for (auto& HitBoxPair : HitCharacter->PlayerHitBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			}
		}
		World->LineTraceSingleByChannel(ConfirmedHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if (ConfirmedHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharactersMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true,false };
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharactersMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false,false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return	FShotgunServerSideRewindResult();
	}

	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		//Storing the position of Hit character's hitboxes in its current position 
		CacheHitBoxPositions(Frame.Character, CurrentFrame);
		//Moving the hit characters position back to the frame where the hit took place
		MoveHitBoxes(Frame.Character, Frame);
		EnableCharactersMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);

		//Adding the frame to the TArray
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		//Enable Collision for the head hitbox.
		UBoxComponent* HeadBox = Frame.Character->PlayerHitBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}
	UWorld* World = GetWorld();
	// checking for headshots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmedHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		
		if (!World) return FShotgunServerSideRewindResult();
		World->LineTraceSingleByChannel(ConfirmedHitResult, TraceStart, TraceEnd, ECC_Visibility);

		// If the line trace hit a player it will be stored in this everzone character variable
		AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(ConfirmedHitResult.GetActor());
		if (!EverzoneCharacter) return FShotgunServerSideRewindResult();
		//If the TMap ShotgunResult.Headshots already has the player from the previous comment add 1 to its value, else then add it to the TMap with the value of 1 as its the first recorded hit.
		if (ShotgunResult.Headshots.Contains(EverzoneCharacter))
		{
			ShotgunResult.Headshots[EverzoneCharacter]++;
		}
		else
		{
			ShotgunResult.Headshots.Emplace(EverzoneCharacter, 1);
		}
	}
	//Enabling collision for all players apart from the head hit box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->PlayerHitBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->PlayerHitBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// checking for bodyshots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmedHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		if (!World) return FShotgunServerSideRewindResult();
		World->LineTraceSingleByChannel(ConfirmedHitResult, TraceStart, TraceEnd, ECC_Visibility);

		// If the line trace hit a player it will be stored in this everzone character variable
		AEverzoneCharacter* EverzoneCharacter = Cast<AEverzoneCharacter>(ConfirmedHitResult.GetActor());
		if (!EverzoneCharacter) return FShotgunServerSideRewindResult();
		//If the TMap ShotgunResult.Headshots already has the player from the previous comment add 1 to its value, else then add it to the TMap with the value of 1 as it's the first recorded hit.
		if (ShotgunResult.Bodyshots.Contains(EverzoneCharacter))
		{
			ShotgunResult.Bodyshots[EverzoneCharacter]++;
		}
		else
		{
			ShotgunResult.Bodyshots.Emplace(EverzoneCharacter, 1);
		}
	}
	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharactersMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	
	return ShotgunResult;
}

void ULagCompensationComponent::CacheHitBoxPositions(AEverzoneCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->PlayerHitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			//Using box info to store the hit character's hitbox information
			FHitBoxInfo BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			//Using the frame package param store the hit information in the frame that it happened in.
			OutFramePackage.HitboxMap.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}
void ULagCompensationComponent::MoveHitBoxes(AEverzoneCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->PlayerHitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitboxMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitboxMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitboxMap[HitBoxPair.Key].BoxExtent);
		}
	}
}
void ULagCompensationComponent::ResetHitBoxes(AEverzoneCharacter* HitCharacter, const FFramePackage& FramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->PlayerHitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitboxMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitboxMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitboxMap[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
void ULagCompensationComponent::EnableCharactersMeshCollision(AEverzoneCharacter* HitCharacter, ECollisionEnabled::Type CollsionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollsionEnabled);
	}
}
void ULagCompensationComponent::SaveFrame() 
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		// Save the current frame by adding it to the double linked list (Frame History)
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		// History Length contains the amount of time stored in the double linked list
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

		while (HistoryLength > MaxRecordTime)// The maximum length of time the double linked list can store is 300 milliseconds
		{
			// Remove the oldest frame and then check History Length's value again
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Blue);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Colour)
{
	//This function is purely to see the effects of server side rewind in the editor. It has no effect on how server side rewind works
	for (auto& BoxInfo : PackageToShow.HitboxMap)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Colour, false, 0.3f);
	}
}
FFramePackage ULagCompensationComponent::GetFrameToCheck(AEverzoneCharacter* HitCharacter, float HitTime)
{
	//null checks
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();

	// Frame package variable that is being used to check for a Hit
	FFramePackage FPackageToCheck;
	bool bShouldToInterpolate = true;
	// Frame History of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		// Too laggy as time no longer exists on the Frame History(Double Linked List)
		return FFramePackage();
	}
	if (NewestHistoryTime <= HitTime)
	{
		FPackageToCheck = History.GetHead()->GetValue();
		bShouldToInterpolate = false;
	}
	if (OldestHistoryTime == HitTime)
	{
		FPackageToCheck = History.GetTail()->GetValue();
		bShouldToInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger;
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetHead();

	while (Older->GetValue().Time > HitTime)//Is Older a more recent time than HitTime?
	{
		//If conditional is true move "older" to check the next node.
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
	}
	if (Older->GetValue().Time == HitTime)
	{
		FPackageToCheck = History.GetTail()->GetValue();
		bShouldToInterpolate = false;
	}
	//once we exit the while loop we know the previous node "older" checked is the most recent node after the hit time
	Younger = Older->GetPrevNode();
	if (bShouldToInterpolate)
	{
		FPackageToCheck = FrameToInterp(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	return FPackageToCheck;
}
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<AEverzoneCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FPackagesToCheck;
	for (AEverzoneCharacter* HitCharacter : HitCharacters)
	{
		FPackagesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FPackagesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResult ULagCompensationComponent::HitScanServerSideRewind(AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FPackageToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FPackageToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult ConfirmHit = HitScanServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (Character && HitCharacter && ConfirmHit.bHitConfirmed && DamageCauser)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(), Character->Controller, DamageCauser, UDamageType::StaticClass());
	}
}
void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(const TArray<AEverzoneCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult ConfirmHit = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	for (auto& EverzoneCharacter : HitCharacters)
	{
		if (Character == nullptr || EverzoneCharacter == nullptr || EverzoneCharacter->GetEquippedWeapon() == nullptr ) continue;
		float TotalDamage = 0;
		if (ConfirmHit.Headshots.Contains(EverzoneCharacter))
		{
			float HeadshotDamage = ConfirmHit.Headshots[EverzoneCharacter] * EverzoneCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += HeadshotDamage;
		}
		if (ConfirmHit.Bodyshots.Contains(EverzoneCharacter))
		{
			float BodyshotDamage = ConfirmHit.Bodyshots[EverzoneCharacter] * EverzoneCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyshotDamage;
		}
		UGameplayStatics::ApplyDamage(EverzoneCharacter, TotalDamage, Character->Controller, EverzoneCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
	}

}
// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFrame();
}



