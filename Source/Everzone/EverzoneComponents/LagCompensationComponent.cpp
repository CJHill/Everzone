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
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Colour)
{
	for (auto& BoxInfo : PackageToShow.HitboxMap)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Colour, false, 0.3f);
	}
}

void ULagCompensationComponent::HitScanServerSideRewind(AEverzoneCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	//null checks
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return;

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
		return;
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

	}
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



