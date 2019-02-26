// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLSkeletalDataAsset.generated.h"

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

private:
	// Generate the bones mesh from this skeletal mesh
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USkeletalMesh* SkeletalMesh;

	// Refresh the data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReloadData;

	// Clear emtpy entries
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bClearEmptyEntries;

	// Remove all previous data mimic button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bClearAllData;
};
