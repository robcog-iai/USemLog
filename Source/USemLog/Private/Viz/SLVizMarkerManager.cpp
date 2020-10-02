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
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const FTransform& Pose, UStaticMesh* SM, const FLinearColor& InColor, ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM, InColor, MaterialType);
	Marker->AddInstance(Pose);
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

// Create a static mesh visual marker at the given poses (use original material)
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarker(const TArray<FTransform>& Poses, UStaticMesh* SM)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM);
	Marker->AddInstances(Poses);
	return Marker;
}

// Create a static mesh visual marker timeline at the given poses
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	float Duration, bool bLoop, float UpdateRate)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM, InColor, MaterialType);
	Marker->AddInstances(Poses, Duration, bLoop, UpdateRate);
	return Marker;
}

// Create a static mesh visual marker timeline at the given poses (use original material)
USLVizStaticMeshMarker* ASLVizMarkerManager::CreateStaticMeshMarkerTimeline(const TArray<FTransform>& Poses, UStaticMesh* SM,
	float Duration, bool bLoop, float UpdateRate)
{
	auto Marker = CreateAndAddNewMarker<USLVizStaticMeshMarker>(this);
	Marker->SetVisual(SM);
	Marker->AddInstances(Poses, Duration, bLoop, UpdateRate);
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
	ESLVizPrimitiveMarkerType PrimitiveType,
	float Size,
	const FLinearColor& InColor,
	ESLVizMaterialType MaterialType)
{
	auto Marker = CreateAndAddNewMarker<USLVizPrimitiveMarker>(this);
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstances(Poses);
	return Marker;
}

// Create a primitive marker timeline at the given poses
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarkerTimeline(const TArray<FTransform>& Poses, 
	ESLVizPrimitiveMarkerType PrimitiveType, float Size, const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	float Duration, bool bLoop, float UpdateRate)
{
	auto Marker = CreateAndAddNewMarker<USLVizPrimitiveMarker>(this);
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstances(Poses, Duration, bLoop, UpdateRate);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
	USkeletalMesh* SkelMesh,
	const TArray<int32>& MaterialIndexes)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	switch (MaterialIndexes.Num())
	{
	case(0):
		Marker->SetVisual(SkelMesh); break;
	case(1):
		Marker->SetVisual(SkelMesh, MaterialIndexes[0]); break;
	default:
		Marker->SetVisual(SkelMesh, MaterialIndexes); break;
	}
	Marker->AddInstance(SkeletalPose);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TPair<FTransform, TMap<int32, FTransform>>& SkeletalPose,
	USkeletalMesh* SkelMesh,
	const FLinearColor& InColor, ESLVizMaterialType MaterialType,
	const TArray<int32>& MaterialIndexes)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	switch (MaterialIndexes.Num())
	{
	case(0):
		Marker->SetVisual(SkelMesh, InColor, MaterialType); break;
	case(1):
		Marker->SetVisual(SkelMesh, MaterialIndexes[0], InColor, MaterialType); break;
	default:
		Marker->SetVisual(SkelMesh, MaterialIndexes, InColor, MaterialType); break;
	}
	Marker->AddInstance(SkeletalPose);
	return Marker;
}

// Create a skeletal mesh based marker at the given poses (use original material)
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	USkeletalMesh* SkelMesh,
	const TArray<int32>& MaterialIndexes)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	switch (MaterialIndexes.Num())
	{
	case(0):
		Marker->SetVisual(SkelMesh); break;
	case(1):
		Marker->SetVisual(SkelMesh, MaterialIndexes[0]); break;
	default:
		Marker->SetVisual(SkelMesh, MaterialIndexes); break;
	}
	Marker->AddInstances(SkeletalPoses);
	return Marker;
}

// Create a skeletal mesh based marker at the given pose
USLVizSkeletalMeshMarker* ASLVizMarkerManager::CreateSkeletalMarker(const TArray<TPair<FTransform, TMap<int32, FTransform>>>& SkeletalPoses,
	USkeletalMesh* SkelMesh,
	const FLinearColor& InColor,
	ESLVizMaterialType MaterialType,
	const TArray<int32>& MaterialIndexes)
{
	auto Marker = CreateAndAddNewMarker<USLVizSkeletalMeshMarker>(this);
	switch (MaterialIndexes.Num())
	{
	case(0):
		Marker->SetVisual(SkelMesh, InColor, MaterialType); break;
	case(1):
		Marker->SetVisual(SkelMesh, MaterialIndexes[0], InColor, MaterialType); break;
	default:
		Marker->SetVisual(SkelMesh, MaterialIndexes, InColor, MaterialType); break;
	}
	Marker->AddInstances(SkeletalPoses);
	return Marker;
}

