// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/Marker/SLVizMarker.h"
#include "Viz/Marker/SLVizPrimitiveMarker.h"
#include "SLVizMarkerManager.generated.h"

// Forward declarations
class USLVizBaseMarker;
class USLVizStaticMeshMarker;
class USLVizSkeletalMeshMarker;

/*
* Spawns and keeps track of markers
*/
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Marker Manager")
class USEMLOG_API ASLVizMarkerManager : public AInfo
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ASLVizMarkerManager();

protected:
	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Clear marker
	void ClearMarker(USLVizMarker* Marker);
	void ClearNewMarker(USLVizBaseMarker* Marker);
	
	// Clear all markers
	void ClearAllMarkers();

	// Create a primitive marker at the given pose
	USLVizPrimitiveMarker* CreatePrimitiveMarker(const FTransform& Pose,
		ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box,
		float Size = .1f,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType MaterialType = ESLVizMarkerMaterialType::Unlit);

	// Create a primitive marker at the given poses
	USLVizPrimitiveMarker* CreatePrimitiveMarker(const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box,
		float Size = .1f,
		const FLinearColor& InColor = FLinearColor::Green,
		ESLVizMarkerMaterialType MaterialType = ESLVizMarkerMaterialType::Unlit);

	// Create marker at the given pose
	USLVizMarker* CreateMarker(const FTransform& Pose, const FSLVizMarkerVisualParams& VisualParams);

	// Create marker at the given poses
	USLVizMarker* CreateMarker(const TArray<FTransform>& Poses, const FSLVizMarkerVisualParams& VisualParams);

	// Create marker at the given skeletal pose
	USLVizMarker* CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, const FSLVizMarkerVisualParams& VisualParams);

	// Create marker at the given skeletal poses
	USLVizMarker* CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, const FSLVizMarkerVisualParams& VisualParams);

protected:
	// Create and register the marker
	USLVizMarker* CreateNewMarker();

protected:
	// Collection of the markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TSet<USLVizMarker*> Markers;


	// Collection of the markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TSet<USLVizBaseMarker*> NewMarkers;
};
