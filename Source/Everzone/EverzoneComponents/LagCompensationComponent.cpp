// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Everzone/Character/EverzoneCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
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
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Colour)
{
	for (auto& BoxInfo : PackageToShow.HitboxMap)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Colour, false, 0.3f);
	}
}

FServerSideRewindResult ULagCompensationComponent::HitScanServerSideRewind(AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	//null checks
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FServerSideRewindResult();

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
		return FServerSideRewindResult();
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
	return ConfirmHit(FPackageToCheck, HitCharacter, TraceStart, HitLocation);
}
// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Blue);
	}
}



