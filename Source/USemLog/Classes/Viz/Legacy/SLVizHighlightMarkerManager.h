// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/Legacy/SLVizHighlightMarker.h"
#include "SLVizHighlightMarkerManager.generated.h"

/*
* Spawns and keeps track of highlight markers
*/
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Highlight Marker Manager")
class USEMLOG_API ASLVizHighlightMarkerManager : public AInfo
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ASLVizHighlightMarkerManager();

protected:
	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Clear hihlight marker
	bool ClearMarker(USLVizHighlightMarker* HighlightMarker);
	
	// Clear all markers
	void ClearAllMarkers();

	/* Highlight markers */
	// Create a highlight marker for the given static mesh component
	USLVizHighlightMarker* CreateHighlightMarker(UStaticMeshComponent* SMC,	
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Create a highlight marker for the given skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC,
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Create a highlight marker for the given bone (material index) skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Create a highlight marker for the given bones (material indexes) skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		const FSLVizHighlightMarkerVisualParams& VisualParams = FSLVizHighlightMarkerVisualParams());

	// Change the visual parameters of the highlight marker
	bool SetVisualParameters(USLVizHighlightMarker* HighlightMarker, const FSLVizHighlightMarkerVisualParams& VisualParams);

protected:
	// Create and register the highlight marker
	USLVizHighlightMarker* CreateHighlightMarkerObject();

protected:
	// Collection of the highlight markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TSet<USLVizHighlightMarker*> HighlightMarkers;
};
