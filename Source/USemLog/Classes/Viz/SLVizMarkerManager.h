// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Viz/SLVizMarker.h"
#include "SLVizMarkerManager.generated.h"

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
	
	// Clear all markers
	void ClearAllMarkers();

	// Create marker at the given pose
	USLVizMarker* CreateMarker(const FTransform& Pose, const FSLVizMarkerVisualParams& VisualParams = FSLVizMarkerVisualParams());

	// Create marker at the given poses
	USLVizMarker* CreateMarker(const TArray<FTransform>& Poses, const FSLVizMarkerVisualParams& VisualParams = FSLVizMarkerVisualParams());

	// Create marker at the given skeletal pose
	USLVizMarker* CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, const FSLVizMarkerVisualParams& VisualParams);

	// Create marker at the given skeletal poses
	USLVizMarker* CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, const FSLVizMarkerVisualParams& VisualParams);



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

protected:
	// Create and register the marker
	USLVizMarker* CreateNewMarker();

protected:
	// Collection of the markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TSet<USLVizMarker*> Markers;
};
