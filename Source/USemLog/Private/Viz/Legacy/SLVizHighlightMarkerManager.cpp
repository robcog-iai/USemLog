// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Legacy/SLVizHighlightMarkerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values for this component's properties
ASLVizHighlightMarkerManager::ASLVizHighlightMarkerManager()
{
	PrimaryActorTick.bCanEverTick = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called when actor removed from game or game ended
void ASLVizHighlightMarkerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearAllMarkers();
}

// Clear hihlight marker
bool ASLVizHighlightMarkerManager::ClearMarker(USLVizHighlightMarker* HighlightMarker)
{
	if (HighlightMarker && HighlightMarker->IsValidLowLevel() && !HighlightMarker->IsPendingKillOrUnreachable())
	{
		//HighlightMarker->ConditionalBeginDestroy();
		HighlightMarker->DestroyComponent();
		if (HighlightMarkers.Remove(HighlightMarker) == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not in the set.."), *FString(__FUNCTION__), __LINE__);
			return false;
		}
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not valid.."), *FString(__FUNCTION__), __LINE__);
		if (HighlightMarkers.Remove(HighlightMarker) == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Removed an invalid marker from the set.."), *FString(__FUNCTION__), __LINE__);
		}
		return false;
	}
}

// Clear all markers
void ASLVizHighlightMarkerManager::ClearAllMarkers()
{
	for (const auto& HM : HighlightMarkers)
	{
		if (HM->IsValidLowLevel() && !HM->IsPendingKillOrUnreachable())
		{
			//HM->ConditionalBeginDestroy();
			HM->DestroyComponent();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not valid.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	HighlightMarkers.Empty();
}

/* Highlight markers */
// Create a highlight marker for the given static mesh component
USLVizHighlightMarker* ASLVizHighlightMarkerManager::CreateHighlightMarker(UStaticMeshComponent* SMC, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	USLVizHighlightMarker* HighlightMarker = CreateHighlightMarkerObject();
	HighlightMarker->Set(SMC, VisualParams);
	return HighlightMarker;
}

// Create a highlight marker for the given skeletal mesh component
USLVizHighlightMarker* ASLVizHighlightMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	USLVizHighlightMarker* HighlightMarker = CreateHighlightMarkerObject();
	HighlightMarker->Set(SkMC, VisualParams);
	return HighlightMarker;
}

// Create a highlight marker for the given bone (material index) skeletal mesh component
USLVizHighlightMarker* ASLVizHighlightMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	USLVizHighlightMarker* HighlightMarker = CreateHighlightMarkerObject();
	HighlightMarker->Set(SkMC, MaterialIndex, VisualParams);
	return HighlightMarker;
}

// Create a highlight marker for the given bones (material indexes) skeletal mesh component
USLVizHighlightMarker * ASLVizHighlightMarkerManager::CreateHighlightMarker(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	USLVizHighlightMarker* HighlightMarker = CreateHighlightMarkerObject();
	HighlightMarker->Set(SkMC, MaterialIndexes, VisualParams);
	return HighlightMarker;
}

// Change the visual parameters of the highlight marker
bool ASLVizHighlightMarkerManager::SetVisualParameters(USLVizHighlightMarker* HighlightMarker, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	if (HighlightMarker && HighlightMarker->IsValidLowLevel() && !HighlightMarker->IsPendingKillOrUnreachable())
	{
		if (HighlightMarkers.Contains(HighlightMarker))
		{
			return HighlightMarker->UpdateVisualParameters(VisualParams);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Requested marker is not found.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	return false;
}


// Create and register the highlight marker
USLVizHighlightMarker* ASLVizHighlightMarkerManager::CreateHighlightMarkerObject()
{
	USLVizHighlightMarker* HighlightMarker = NewObject<USLVizHighlightMarker>(this);
	HighlightMarker->RegisterComponent();
	AddInstanceComponent(HighlightMarker); // Makes it appear in the editor
	HighlightMarkers.Add(HighlightMarker);
	return HighlightMarker;
}