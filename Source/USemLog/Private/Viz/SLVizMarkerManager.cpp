// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizMarkerManager.h"
#include "Viz/Markers/SLVizBaseMarker.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
ASLVizMarkerManager::ASLVizMarkerManager()
{
	PrimaryActorTick.bCanEverTick = false;
	// Add a default root component to have the markers attached to something
	// commented out since it is not being attached ATM
	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ManagerRootComponent"));
}

// Called when actor removed from game or game ended
void ASLVizMarkerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearAllMarkers();
}

// Clear marker
void ASLVizMarkerManager::ClearMarker(USLVizBaseMarker* Marker)
{
	// Destroy only if managed by this manager
	if (Markers.Remove(Marker) > 0)
	{
		if (Marker && Marker->IsValidLowLevel() && !Marker->IsPendingKillOrUnreachable())
		{
			Marker->DestroyComponent();
		}
	}
}

// Clear all markers
void ASLVizMarkerManager::ClearAllMarkers()
{	
	for (auto Marker : Markers)
	{
		//RemoveOwnedComponent(Marker);
		//RemoveInstanceComponent(Marker);
		if (Marker && Marker->IsValidLowLevel() && !Marker->IsPendingKillOrUnreachable())
		{
			Marker->DestroyComponent();
		}
	}
	Markers.Empty();
}

// Create a static mesh visual marker at the given pose (use original material)
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const FTransform& Pose, UStaticMesh* SM)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM);
	Marker->AddInstance(Pose);
	return Marker;
}

// Create a static mesh visual marker at the given pose
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const FTransform& Pose, UStaticMesh* SM,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM, InColor, MaterialType);
	Marker->AddInstance(Pose);
	return Marker;
}

// Create a static mesh visual marker at the given poses (use original material)
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const TArray<FTransform>& Poses, UStaticMesh* SM)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM);
	Marker->AddInstances(Poses);
	return Marker;
}

// Create a static mesh visual marker at the given poses
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const TArray<FTransform>& Poses, UStaticMesh* SM,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM, InColor, MaterialType);
	Marker->AddInstances(Poses);
	return Marker;
}

// Create a static mesh visual marker timeline at the given poses
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM, InColor, MaterialType);
	Marker->AddInstances(Poses, TimelineParams);
	return Marker;
}

// Create a static mesh visual marker timeline at the given poses (use original material)
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
	const FSLVizTimelineParams& TimelineParams)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM);
	Marker->AddInstances(Poses, TimelineParams);
	return Marker;
}

// Create a primitive marker with one instance
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarker(const FTransform& Pose, 
	ESLVizPrimitiveMarkerType PrimitiveType, 
	float Size, 
	const FLinearColor& InColor, 
	ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizPrimitiveMarker>(this);
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstance(Pose);
	return Marker;
}

// Create a primitive marker with multiple instances
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarker(const TArray<FTransform>& Poses,
	ESLVizPrimitiveMarkerType PrimitiveType, float Size,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizPrimitiveMarker>(this);
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstances(Poses);
	return Marker;
}

// Create a primitive marker timeline at the given poses
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarkerTimeline(const TArray<FTransform>& Poses, 
	ESLVizPrimitiveMarkerType PrimitiveType, float Size, 
	const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	auto Marker = CreateAndAddNewMarker<USLVizPrimitiveMarker>(this);
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstances(Poses, TimelineParams);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
	USkeletalMesh* SkelMesh)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	Marker->SetVisual(SkelMesh);
	Marker->AddInstance(SkeletalPose);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
	USkeletalMesh* SkelMesh, const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	Marker->SetVisual(SkelMesh, InColor, MaterialType);
	Marker->AddInstance(SkeletalPose);
	return Marker;
}

// Create a skeletal mesh based marker at the given poses (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	USkeletalMesh* SkelMesh)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	Marker->SetVisual(SkelMesh);
	Marker->AddInstances(SkeletalPoses);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	USkeletalMesh* SkelMesh, const FLinearColor& InColor,ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);

	Marker->SetVisual(SkelMesh, InColor, MaterialType);
	Marker->AddInstances(SkeletalPoses);
	return Marker;
}

// Create a skeletal mesh based timeline marker at the given poses (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarkerTimeline(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses, USkeletalMesh* SkelMesh, const FSLVizTimelineParams& TimelineParams)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	Marker->SetVisual(SkelMesh);
	Marker->AddInstances(SkeletalPoses, TimelineParams);
	return Marker;
}

// Create a skeletal mesh based timeline marker at the given poses
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarkerTimeline(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	USkeletalMesh* SkelMesh,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	const FSLVizTimelineParams& TimelineParams)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	Marker->SetVisual(SkelMesh, InColor, MaterialType);
	Marker->AddInstances(SkeletalPoses, TimelineParams);
	return Marker;
}

// Create a skeletal bone visual marker at the given pose (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalBoneMarker(const FTransform& Pose, USkeletalMesh* SkelMesh, int32 MaterialIndex)
{
	// TODO
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);

	auto Marker = CreateAndAddNewMarker<USLVizSkeletalBoneMeshMarker>(this);
	Marker->SetVisual(SkelMesh, MaterialIndex);
	//Marker->AddInstance(Pose);
	return Marker;
}

// Create a skeletal bone visual marker at the given pose
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalBoneMarker(const FTransform& Pose, USkeletalMesh* SkelMesh, int32 MaterialIndex,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalBoneMeshMarker>(this);
	Marker->SetVisual(SkelMesh, MaterialIndex, InColor, MaterialType);
	//Marker->AddInstance(Pose);
	return Marker;
}

// Create a skeletal bone visual marker at the given poses (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalBoneMarker(const TArray<FTransform>& Poses, USkeletalMesh* SkelMesh, int32 MaterialIndex)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	return nullptr;
}

// Create a skeletal bone visual marker at the given poses
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalBoneMarker(const TArray<FTransform>& Poses, USkeletalMesh* SkelMesh, int32 MaterialIndex, const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	return nullptr;
}

