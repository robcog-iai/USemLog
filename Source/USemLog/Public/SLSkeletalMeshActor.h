// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLSkeletalMapDataAsset.h"
#include "SLSkeletalMeshActor.generated.h"

/**
 * Subclass of ASkeletalMeshActor that contains a data asset component
 * that holds the mapping of skeletal bone names to their semantic names
 */
UCLASS()
class USEMLOG_API ASLSkeletalMeshActor : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	// Get the semantic mapping data asset
	USLSkeletalMapDataAsset* GetSkeletalMapDataAsset() const { return SkeletalMapDataAsset; }

private:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USLSkeletalMapDataAsset* SkeletalMapDataAsset;
};
