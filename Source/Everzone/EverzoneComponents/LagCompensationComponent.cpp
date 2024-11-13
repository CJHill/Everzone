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
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& PackageToShow, const FColor& Colour)
{
	for (auto& BoxInfo : PackageToShow.HitboxMap)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Colour, false, 0.3f);
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



