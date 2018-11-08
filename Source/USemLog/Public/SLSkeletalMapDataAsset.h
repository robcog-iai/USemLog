// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLSkeletalMapDataAsset.generated.h"

/**
 * Data asset containing the mapping of skeletal bones to their semantic names
 */
UCLASS(ClassGroup = (SL), meta = (DisplayName = "SL Skeletal Map Data Asset"))
class USEMLOG_API USLSkeletalMapDataAsset : public UDataAsset
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Map of bones to their class names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FString> BoneClassMap;	

private:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	UPhysicsAsset* PhysicsAsset;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeleton* Skeleton;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeletalMesh* SkeletalMesh;
};
