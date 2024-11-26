// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//defining the collision channels here so that it isn't simply called GameTraceChannel as that would be a poor naming convention
// must include this header file where appropriate to notice this definition
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1 
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2