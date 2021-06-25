// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizStructs.h"
#include "SLVizHighlightManager.generated.h"

// Forward declarations
class USLVizAssets;
class UMeshComponent;
class UMaterialInterface;

/**
 * Stores the original materials for re-applying them
 * and the dynamic material for allowing dynamic color updates
 */
USTRUCT()
struct USEMLOG_API FSLVizHighlightData
{
	GENERATED_BODY()

	// Array of the original materials
	UPROPERTY()
	TArray<UMaterialInterface*> OriginalMaterials;

	// Material slots to which to apply the visual parameters (if empty, apply to all slots)
	UPROPERTY()
	TArray<int32> MaterialSlots;

	// Default ctor
	FSLVizHighlightData() {};

	// Init ctor
	FSLVizHighlightData(const TArray<UMaterialInterface*>& InMaterials) : OriginalMaterials(InMaterials) {};

	// Init ctor
	FSLVizHighlightData(const TArray<UMaterialInterface*>& InMaterials, const TArray<int32>& InMaterialSlots) 
		: OriginalMaterials(InMaterials), MaterialSlots(InMaterialSlots){};
};


/**
 * Manages highliting of individuals without the use of markers
 * keeps track of modified and original materials of actors
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Highlight Manager")
class USEMLOG_API ASLVizHighlightManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	ASLVizHighlightManager();

protected:
	// Do any object-specific cleanup required immediately after loading an object. (called only once when loading the map)
	virtual void PostLoad() override;

	// When an actor is created in the editor or during gameplay, this gets called right before construction. This is not called for components loaded from a level. 
	virtual void PostActorCreated() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending 
	virtual void Destroyed() override;

	// Load container with the vizual assets
	bool LoadAssetsContainer();

	// Callback function registered with global world delegates to reset materials to their original values
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	// Make sure the original materials are applied if the manager is destroyed or the level closed etc.
	void RestoreOriginalMaterials();

public:
	// Highlight the given mesh component
	void Highlight(UMeshComponent* MC, const FSLVizVisualParams& VisualParams = FSLVizVisualParams());

	// Update the visual of the given mesh component
	void UpdateHighlight(UMeshComponent* MC, const FSLVizVisualParams& VisualParams);

	// Clear highlight of a static mesh
	void ClearHighlight(UMeshComponent* MC);

	// Clear all highlights
	void ClearAllHighlights();

private:
	// Bind delegates
	void BindDelgates();

	// Remove any bound delegates
	void RemoveDelegates();

	// Create a dynamic material instance
	UMaterialInstanceDynamic* CreateTransientMID(ESLVizMaterialType InMaterialType);


protected:
	// List of the highlighted static meshes with their original materials
	//UPROPERTY()
	TMap<UMeshComponent*, FSLVizHighlightData> HighlightedStaticMeshes;

private:
	// Viz assets container
	USLVizAssets* VizAssetsContainer;

	/* Constants */
	static constexpr auto AssetsContainerPath = TEXT("SLVizAssets'/USemLog/Viz/SL_VizAssetsContainer.SL_VizAssetsContainer'");
};
