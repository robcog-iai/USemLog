// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstance.h"
#include "SLSkeletalDataAsset.h"
#include "SLSkeletalDataComponent.generated.h"

/**
 * Bone semantic data structure
 */
USTRUCT()
struct FSLBoneData
{
	GENERATED_BODY()

	// Semantic class name
	UPROPERTY(EditAnywhere)
	FString Class;

	// Color mask
	UPROPERTY(EditAnywhere)
	FString MaskColorHex;

	// Mask material instance 
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MaskMaterialInstance;	
};

/**
 * Stores the semantic skeletal data of its parent skeletal mesh component
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USEMLOG_API USLSkeletalDataComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLSkeletalDataComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

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
	TMap<FName, FSLBoneData> BonesData;

private:
	// Load the bones semantic information from this data asset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USLSkeletalDataAsset* LoadFromDataAsset;

	// Load new data without removing previous one button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReloadData;

	// Remove all previous data mimic button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bClearAllData;
};
