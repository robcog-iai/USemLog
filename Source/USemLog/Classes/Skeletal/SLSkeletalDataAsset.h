// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLSkeletalDataAsset.generated.h"

///**
// * Data stored in the bones array
// */
//USTRUCT()
//struct FSLSkeletalBoneData
//{
//	GENERATED_BODY()
//
//	UPROPERTY(EditAnywhere)
//	FName BoneFName;
//
//	UPROPERTY(EditAnywhere)
//	int32 BoneIndex;
//
//	UPROPERTY(EditAnywhere)
//	FString ClassName;
//};

/**
 * Data asset containing the mapping of skeletal bones to their semantic names
 */
UCLASS(ClassGroup = (SL), meta = (DisplayName = "SL Skeletal Map Data Asset"))
class USEMLOG_API USLSkeletalDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Ctor
	USLSkeletalDataAsset();

protected:
#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
	// Update the data
	void LoadData();

public:
	// Map of bones to their class names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FString> BoneClasses;

	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<int32, FString> BoneIndexClass;

private:
	// Generate the bones mesh from this skeletal mesh
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	class USkeletalMesh* SkeletalMesh;

	// Refresh the data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReloadData;

	// Remove all previous data mimic button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bClearAllData;
};
