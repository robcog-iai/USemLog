// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "SLVizHighlightMarker.generated.h"

// Forward declarations
class USLVizAssets;

/**
 * Highlight material types
 */
UENUM()
enum class ESLVizHighlightMarkerMaterialType : uint8
{
	NONE				UMETA(DisplayName = "NONE"),
	Additive			UMETA(DisplayName = "Additive"),
	Translucent			UMETA(DisplayName = "Translucent"),
};

/**
 * Highlight marker parameters
 */
USTRUCT()
struct FSLVizHighlightMarkerVisualParams
{
	GENERATED_BODY();

	// Highlight color
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	// Material type
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizHighlightMarkerMaterialType MaterialType = ESLVizHighlightMarkerMaterialType::Additive;
};

/**
 * Highlights a static or skeletal mesh by creating a clone in the same position
 */
UCLASS()
class USEMLOG_API USLVizHighlightMarker : public USceneComponent
{
	GENERATED_BODY()
	
public:
	// Constructor
	USLVizHighlightMarker();

	// Highlight the given static mesh by creating a clone
	void Set(UStaticMeshComponent* SMC, const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Highlight the given skeletal mesh by creating a clone
	void Set(USkeletalMeshComponent* SkMC, const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Highlight the given bone (material index) by creating a clone
	void Set(USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Highlight the given bones (material indexes) by creating a clone
	void Set(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Set the visual parameters
	bool UpdateVisualParameters(const FSLVizHighlightMarkerVisualParams& VisualParams);

	// Call this if you want to notify the owner (manager) of the destruction
	bool DestroyThroughManager();

	//~ Begin ActorComponent Interface
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	//~ End ActorComponent Interface

protected:
	// Load assets container
	bool LoadAssetsContainer();

private:
	// Set the static mesh component
	bool SetStaticMeshComponent(UStaticMeshComponent* SMC);

	// Clear the static mesh component
	void ClearStaticMeshComponent();

	// Set the skeletal mesh component
	bool SetSkeletalMeshComponent(USkeletalMeshComponent* SkMC);

	// Clear the skeletal mesh component
	void ClearSkeletalMeshComponent();

	// Set the dynamic material
	void SetDynamicMaterial(const FSLVizHighlightMarkerVisualParams& VisualParams);

	// Clear the dynamic material
	void ClearDynamicMaterial();

private:
	// Used as a clone if a static mesh component will be highlighted
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UStaticMeshComponent* HighlightSMC;

	// Used as a clone if a skeletal mesh component will be highlighted
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UPoseableMeshComponent* HighlightSkelMC;

	// Used when updating the visual parameters
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TArray<int32> SkeletalMaterialIndexes;

	// Dynamic material
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	UMaterialInstanceDynamic* DynamicMaterial;

	// Current material type
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ESLVizHighlightMarkerMaterialType MaterialType;

	// Assets container
	USLVizAssets* VizAssetsContainer;

	/* Constants */
	static constexpr auto AssetsContainerPath = TEXT("SLVizAssets'/USemLog/Viz/SL_VizAssetsContainer.SL_VizAssetsContainer'");
};
