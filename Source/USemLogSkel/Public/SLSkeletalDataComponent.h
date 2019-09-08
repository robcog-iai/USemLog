// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "SLStructs.h"
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

	//// Mask material instance 
	//UPROPERTY(EditAnywhere)
	//UMaterialInterface* MaskMaterial;

	// Mask material index
	UPROPERTY(EditAnywhere)
	int32 MaskMaterialIndex;

	//// Default constructor
	//FSLBoneData() : Class(""), MaskColorHex(""), MaskMaterialIndex(INDEX_NONE) {};

	// Checks if the structure has been set
	bool IsSet() const { return !Class.IsEmpty(); };

	// Get result as string
	FString ToString() const
	{
		return FString::Printf(TEXT("Class:%s; MaskColorHex:%s; MaskMaterialIndex:%d"), *Class, *MaskColorHex, MaskMaterialIndex);
	}
};

/**
 * Stores the semantic skeletal data of its parent skeletal mesh component
 * SceneComponent so it can be added to skeletal components that are not inheriting from a SkeletalMeshActor
 */
UCLASS( ClassGroup=(SL), meta=(BlueprintSpawnableComponent), hidecategories = (HLOD, Cooking, Transform, Animation, Mesh, Materials), DisplayName="SL Skeletal Data")
class USEMLOGSKEL_API USLSkeletalDataComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USLSkeletalDataComponent();

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	// Init the component for runtime, returns true if it was already init, or init was successful
	bool Init();

	// Check if the component is init (and valid)
	bool IsInit() const { return bInit; };

private:
	// Update the data from the data asset
	void LoadFromDataAsset();

	// Refresh the values (update material index, remove invalid data)
	void Refresh();

	// Clear data
	void ClearData();

	// Set the skeletal parent, returns true if successful or is already set
	bool SetSkeletalParent();

	// Set the semantic parent and its data, returns true if successful or is already set
	bool SetSemanticOwnerData();

	// Set data for all the bones (empty for the ones without semantics)
	void SetDataForAllBones();

public:
	// Map of bones to their class names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FSLBoneData> SemanticBonesData;

	// All bones map
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	TMap<FName, FSLBoneData> AllBonesData;

	// The attach parent skeletal mesh
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	USkeletalMeshComponent* SkeletalMeshParent;

	// Semantic owner
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	UObject* SemanticOwner;

	// Semantic data of the owner	
	FSLEntity OwnerSemanticData;

private:
	// Flag marking the component as init (and valid) for runtime 
	bool bInit;

	// Load the bones semantic information from the skeletal data asset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	USLSkeletalDataAsset* SkeletalDataAsset;

	// Clear and re-load data
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bReloadFromDataAssetButton;

	// Refresh the values (update material index, remove invalid data)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bRefresh;

	// Remove all previous data mimic button
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bClearAllDataButton;
};
