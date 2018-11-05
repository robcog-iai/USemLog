// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLSkeletalMeshMapping.generated.h"

/**
 * Data asset containing the mapping of skeletal bones to their semantic names
 */
UCLASS()
class USEMLOG_API USLSkeletalMeshMapping : public UDataAsset
{
	GENERATED_BODY()

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	UPhysicsAsset* PhysicsAsset;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeleton* Skeleton;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FString, FString> BoneSemanticNameMap;	
};
