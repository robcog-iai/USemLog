// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizMarker.h"
#include "SLVizMarkerManager.generated.h"

// Forward declarations
class USLVizHighlightMarker;

/*
* Spawns and keeps track of markers
*/
UCLASS()
class USEMLOG_API ASLVizMarkerManager : public AInfo
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ASLVizMarkerManager();

	// Clear marker
	void ClearMarker(USLVizMarker* Marker);

	// Clear hihlight marker
	void ClearMarker(USLVizHighlightMarker* HighlightMarker);
	
	// Clear all markers
	void ClearAllMarkers();

	/* Primitive static mesh visual markers */
	// Create marker at location
	USLVizMarker* CreateMarker(const FVector& Location,
		ESLVizMarkerType Type = ESLVizMarkerType::Box, 
		const FVector& Scale = FVector(0.1), 
		const FLinearColor& Color = FLinearColor::Green, 
		bool bUnlit = false);

	// Create marker at pose
	USLVizMarker* CreateMarker(const FTransform& Pose,
		ESLVizMarkerType Type = ESLVizMarkerType::Box,
		const FVector& Scale = FVector(0.1),
		const FLinearColor& Color = FLinearColor::Green,
		bool bUnlit = false);

	// Create a marker array at the given locations
	USLVizMarker* CreateMarker(const TArray<FVector>& Locations,
		ESLVizMarkerType Type = ESLVizMarkerType::Box,
		const FVector& Scale = FVector(0.1),
		const FLinearColor& Color = FLinearColor::Green,
		bool bUnlit = false);

	// Create a marker array at the given poses
	USLVizMarker* CreateMarker(const TArray<FTransform>& Poses,
		ESLVizMarkerType Type = ESLVizMarkerType::Box,
		const FVector& Scale = FVector(0.1),
		const FLinearColor& Color = FLinearColor::Green,
		bool bUnlit = false);


	/* Static mesh visual markers */
	// Create marker at location
	USLVizMarker* CreateMarker(const FVector& Location, UStaticMeshComponent* SMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create marker at pose
	USLVizMarker* CreateMarker(const FTransform& Pose, UStaticMeshComponent* SMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create a marker array at the given locations
	USLVizMarker* CreateMarker(const TArray<FVector>& Locations, UStaticMeshComponent* SMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create a marker array at the given poses
	USLVizMarker* CreateMarker(const TArray<FTransform>& Poses, UStaticMeshComponent* SMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);


	/* Skeletal mesh visual markers */
	// Create skeletal marker 
	USLVizMarker* CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose,
		USkeletalMeshComponent* SkMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create skeletal marker array at the given poses
	USLVizMarker* CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses,
		USkeletalMeshComponent* SkMC,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create skeletal marker array for the given bone (material index) at the given pose
	USLVizMarker* CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose,
		USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create skeletal marker array for the given bone (material index) at the given poses
	USLVizMarker* CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses,
		USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create skeletal marker array for the given bones (material indexs) at the given pose
	USLVizMarker* CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose,
		USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);

	// Create skeletal marker array for the given bones (material indexs) at the given poses
	USLVizMarker* CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses,
		USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		bool bUseOriginalMaterials = true, const FLinearColor& Color = FLinearColor::Green, bool bUnlit = false);


	/* Highlight markers */
	// Create a highlight marker for the given static mesh component
	USLVizHighlightMarker* CreateHighlightMarker(UStaticMeshComponent* SMC,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Create a highlight marker for the given skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Create a highlight marker for the given bone (material index) skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC, int32 MaterialIndex,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

	// Create a highlight marker for the given bones (material indexes) skeletal mesh component
	USLVizHighlightMarker* CreateHighlightMarker(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes,
		const FLinearColor& Color = FLinearColor::Green, ESLVizHighlightMarkerType Type = ESLVizHighlightMarkerType::Additive);

protected:
	// Create and register the marker
	USLVizMarker* CreateNewMarker();

	// Create and register the highlight marker
	USLVizHighlightMarker* CreateNewHighlightMarker();

protected:
	// Collection of the markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TSet<USLVizMarker*> Markers;

	// Collection of the highlight markers
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TSet<USLVizHighlightMarker*> HighlightMarkers;
};
