// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Viz/Markers/SLVizPrimitiveMarker.h"
#include "Viz/Markers/SLVizStaticMeshMarker.h"
#include "Viz/Markers/SLVizSkeletalMeshMarker.h"
#include "Viz/Markers/SLVizSkeletalBoneMeshMarker.h"
#include "SLVizMarkerManager.generated.h"


/*
* Spawns and keeps track of markers
* (AActor since AInfo & components seems not to be rendered during runtime)
*/
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Marker Manager")
class USEMLOG_API ASLVizMarkerManager : public AActor
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
	void ClearMarker(USLVizBaseMarker* Marker);
	
	// Clear all markers
	void ClearAllMarkers();


	/* Static mesh markers */
	// Create a static mesh visual marker at the given pose (use original material)
	USLVizStaticMeshMarker* CreateStaticMeshMarker(const FTransform& Pose, UStaticMesh* SM);

	// Create a static mesh visual marker at the given pose
	USLVizStaticMeshMarker* CreateStaticMeshMarker(const FTransform& Pose, UStaticMesh* SM,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a static mesh visual marker at the given poses (use original material)
	USLVizStaticMeshMarker* CreateStaticMeshMarker(const TArray<FTransform>& Poses,	UStaticMesh* SM);

	// Create a static mesh visual marker at the given poses
	USLVizStaticMeshMarker* CreateStaticMeshMarker(const TArray<FTransform>& Poses,	UStaticMesh* SM,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a static mesh visual marker timeline at the given poses
	USLVizStaticMeshMarker* CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType, 
		const FSLVizTimelineParams& TimelineParams);

	// Create a static mesh visual marker timeline at the given poses (use original material)
	USLVizStaticMeshMarker* CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
		const FSLVizTimelineParams& TimelineParams);


	/* Primitive mesh markers */
	// Create a primitive marker at the given pose
	USLVizPrimitiveMarker* CreatePrimitiveMarker(const FTransform& Pose,
		ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box, float Size = .1f,
		const FLinearColor& InColor = FLinearColor::Green, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a primitive marker at the given poses
	USLVizPrimitiveMarker* CreatePrimitiveMarker(const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box, float Size = .1f,
		const FLinearColor& InColor = FLinearColor::Green, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a primitive marker timeline at the given poses
	USLVizPrimitiveMarker* CreatePrimitiveMarkerTimeline(const TArray<FTransform>& Poses,
		ESLVizPrimitiveMarkerType PrimitiveType, float Size,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);


	/* Skeletal mesh markers */
	// Create a skeletal mesh based marker at the given pose (use original material)
	USLVizSkeletalMeshMarker* CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
		USkeletalMesh* SkelMesh);

	// Create a skeletal mesh based marker at the given pose
	USLVizSkeletalMeshMarker* CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
		USkeletalMesh* SkelMesh, 
		const FLinearColor& InColor, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a skeletal mesh based marker at the given poses (use original material)
	USLVizSkeletalMeshMarker* CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		USkeletalMesh* SkelMesh);

	// Create a skeletal mesh based marker at the given poses
	USLVizSkeletalMeshMarker* CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		USkeletalMesh* SkelMesh,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit);

	// Create a skeletal mesh based timeline marker at the given poses (use original material)
	USLVizSkeletalMeshMarker* CreateSkeletalMarkerTimeline(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		USkeletalMesh* SkelMesh,
		const FSLVizTimelineParams& TimelineParams);

	// Create a skeletal mesh based timeline marker at the given poses
	USLVizSkeletalMeshMarker* CreateSkeletalMarkerTimeline(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
		USkeletalMesh* SkelMesh,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType,
		const FSLVizTimelineParams& TimelineParams);


	/* Skeletal mesh (bone) markers */
	// Create a skeletal bone visual marker at the given pose (use original material)
	USLVizSkeletalMeshMarker* CreateSkeletalBoneMarker(const FTransform& Pose, USkeletalMesh* SkelMesh, int32 MaterialIndex);

	// Create a skeletal bone visual marker at the given pose
	USLVizSkeletalMeshMarker* CreateSkeletalBoneMarker(const FTransform& Pose, USkeletalMesh* SkelMesh, int32 MaterialIndex,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType);

	// Create a skeletal bone visual marker at the given poses (use original material)
	USLVizSkeletalMeshMarker* CreateSkeletalBoneMarker(const TArray<FTransform>& Poses, USkeletalMesh* SkelMesh, int32 MaterialIndex);

	// Create a skeletal bone visual marker at the given poses
	USLVizSkeletalMeshMarker* CreateSkeletalBoneMarker(const TArray<FTransform>& Poses, USkeletalMesh* SkelMesh, int32 MaterialIndex,
		const FLinearColor& InColor, ESLVizMaterialType MaterialType);


private:
	// Create and store marker helper function
	template <class T>
	T* CreateAndAddNewMarker(UObject* Outer)
	{
		T* Marker = NewObject<T>(Outer);
		Marker->RegisterComponent();
		//Marker->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		// Removed because they are causing the components to remain in the actors components when changing maps
		//AddInstanceComponent(Marker); // Makes it appear in the editor
		//AddOwnedComponent(Marker);
		Markers.Add(Marker);
		return Marker;
	}

protected:
	// Collection of the markers
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	TSet<USLVizBaseMarker*> Markers;
};
