// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
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
	UMaterialInterface* MaskMaterial;
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

public:
	// Init the component for runtime, returns true if it was already init, or initi was succesful
	bool Init();

	// Check if the component is init (and valid)
	bool IsInit() const { return bInit; };
	
	// Get the skeletal mesh parent
	USkeletalMeshComponent* GetSkeletalMeshParent() const { return SkeletalMeshParent; };

	// Get the semantic data
	TSharedPtr<FSLObject> GetOwnerSemanticData() const { return OwnerSemanticData; };

private:
#if WITH_EDITOR
	// Update the data
	void LoadData();

	// Clear data
	void ClearData(bool bIncludeSkeletal = true);
#endif // WITH_EDITOR

	// Set the skeletal parent, returns true if successful or is already set
	bool SetSkeletalParent();

	// Set the semantic parent, returns true if successful or is already set
	bool SetOwnerSemanticData();

public:
	// Map of bones to their class names
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TMap<FName, FSLBoneData> BonesData;

private:
	// Flag marking the component as init (and valid) for runtime 
	bool bInit;

	// The attach parent skeletal mesh
	USkeletalMeshComponent* SkeletalMeshParent;

	// Semantic owner
	UObject* SemanticOwner;

	// Semantic data of the owner
	TSharedPtr<FSLObject> OwnerSemanticData;

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
