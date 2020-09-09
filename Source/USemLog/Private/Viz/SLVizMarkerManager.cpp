// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizMarkerManager.h"
#include "Viz/Marker/SLVizBaseMarker.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
ASLVizMarkerManager::ASLVizMarkerManager()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called when actor removed from game or game ended
void ASLVizMarkerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearAllMarkers();
}

// Clear marker
void ASLVizMarkerManager::ClearMarker(USLVizMarker* Marker)
{
	if (Marker->IsValidLowLevel() && !Marker->IsPendingKillOrUnreachable())
	{
		//Marker->ConditionalBeginDestroy();
		Marker->DestroyComponent();
		if (Markers.Remove(Marker) == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not in the set.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not valid.."), *FString(__FUNCTION__), __LINE__);
		if (Markers.Remove(Marker) == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Removed an invalid marker from the set.."), *FString(__FUNCTION__), __LINE__);
		}
	}
}

// Clear marker
void ASLVizMarkerManager::ClearNewMarker(USLVizBaseMarker* Marker)
{
	// Destroy only if managed by this manager
	if (NewMarkers.Remove(Marker) > 0)
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
	for (const auto& M : Markers)
	{
		if (M->IsValidLowLevel() && !M->IsPendingKillOrUnreachable())
		{
			M->DestroyComponent();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not valid.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	Markers.Empty();

	for (auto Marker : NewMarkers)
	{
		if (Marker && Marker->IsValidLowLevel() && !Marker->IsPendingKillOrUnreachable())
		{
			Marker->DestroyComponent();
		}
	}
	NewMarkers.Empty();
}

// Create a primitive marker with one instance
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarker(const FTransform& Pose, 
	ESLVizPrimitiveMarkerType PrimitiveType, 
	float Size, 
	const FLinearColor& InColor, 
	ESLVizMarkerMaterialType MaterialType)
{
	USLVizPrimitiveMarker* Marker = NewObject<USLVizPrimitiveMarker>(this);
	Marker->RegisterComponent();
	AddInstanceComponent(Marker); // Makes it appear in the editor
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstance(Pose);
	NewMarkers.Add(Marker);
	return Marker;
}

// Create a primitive marker with multiple instances
USLVizPrimitiveMarker* ASLVizMarkerManager::CreatePrimitiveMarker(const TArray<FTransform>& Poses,
	ESLVizPrimitiveMarkerType PrimitiveType,
	float Size,
	const FLinearColor& InColor,
	ESLVizMarkerMaterialType MaterialType)
{
	USLVizPrimitiveMarker* Marker = NewObject<USLVizPrimitiveMarker>(this);
	Marker->RegisterComponent();
	AddInstanceComponent(Marker); // Makes it appear in the editor
	Marker->SetVisual(PrimitiveType, Size, InColor, MaterialType);
	Marker->AddInstances(Poses);
	NewMarkers.Add(Marker);
	return Marker;
}

// Create marker at the given pose
USLVizMarker* ASLVizMarkerManager::CreateMarker(const FTransform& Pose, const FSLVizMarkerVisualParams& VisualParams)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->SetVisual(VisualParams);
	Marker->Add(Pose);
	return Marker;
}

// Create marker at the given poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<FTransform>& Poses, const FSLVizMarkerVisualParams& VisualParams)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->SetVisual(VisualParams);
	Marker->Add(Poses);
	return Marker;
}

// Create marker at the given skeletal poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose, const FSLVizMarkerVisualParams& VisualParams)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->SetVisual(VisualParams);
	Marker->Add(SkeletalPose);
	return Marker;
}

// Create marker at the given skeletal poses
USLVizMarker* ASLVizMarkerManager::CreateMarker(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses, const FSLVizMarkerVisualParams& VisualParams)
{
	USLVizMarker* Marker = CreateNewMarker();
	Marker->SetVisual(VisualParams);
	Marker->Add(SkeletalPoses);
	return Marker;
}

// Create and register the marker
USLVizMarker* ASLVizMarkerManager::CreateNewMarker()
{
	USLVizMarker* Marker = NewObject<USLVizMarker>(this);
	Marker->RegisterComponent();
	AddInstanceComponent(Marker); // Makes it appear in the editor
	Markers.Add(Marker);
	return Marker;
}
