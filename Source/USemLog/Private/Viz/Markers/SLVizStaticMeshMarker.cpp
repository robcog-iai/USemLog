// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Markers/SLVizStaticMeshMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

// Constructor
USLVizStaticMeshMarker::USLVizStaticMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	ISMC = nullptr;
	TimelineIndex = INDEX_NONE;
	bLoopTimeline = false;
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLVizStaticMeshMarker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	int32 NumInstancesToDraw = (DeltaTime * TimelinePoses.Num()) / TimelineDuration;
	if (TimelineIndex + NumInstancesToDraw < TimelinePoses.Num())
	{
		// It is safe to add all instances
		while (NumInstancesToDraw > 0)
		{
			AddInstanceChecked(TimelinePoses[TimelineIndex]);
			TimelineIndex++;
			NumInstancesToDraw--;
		}
	}
	else
	{
		// Reached end of the poses array, add remaining values
		while (TimelinePoses.IsValidIndex(TimelineIndex))
		{
			AddInstanceChecked(TimelinePoses[TimelineIndex]);
			TimelineIndex++;
		}

		// Check if the timeline should be repeated or stopped
		if (bLoopTimeline)
		{
			ISMC->ClearInstances();
			TimelineIndex = 0;
		}
		else
		{
			ClearTimelineData();
		}
	}
}

// Set the visual properties of the instanced mesh using the mesh original materials
void USLVizStaticMeshMarker::SetVisual(UStaticMesh* SM)
{
	// Clear any previous data
	Reset();

	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		ISMC = NewObject<UInstancedStaticMeshComponent>(this);
		ISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ISMC->bSelectable = false;
		//ISMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		ISMC->RegisterComponent();
	}

	// Set the visual mesh
	ISMC->SetStaticMesh(SM);
}

// Set the visual properties of the instanced mesh
void USLVizStaticMeshMarker::SetVisual(UStaticMesh* SM, const FLinearColor& InColor, ESLVizMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		ISMC = NewObject<UInstancedStaticMeshComponent>(this);
		ISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ISMC->bSelectable = false;
		//ISMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		ISMC->RegisterComponent();
	}

	// Set the visual mesh
	ISMC->SetStaticMesh(SM);

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < ISMC->GetNumMaterials(); ++MatIdx)
	{
		ISMC->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Update the visual mesh type
void USLVizStaticMeshMarker::UpdateStaticMesh(UStaticMesh* SM)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ISMC->SetStaticMesh(SM);

	// Apply dynamic material value
	if (DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		for (int32 MatIdx = 0; MatIdx < ISMC->GetNumMaterials(); ++MatIdx)
		{
			ISMC->SetMaterial(MatIdx, DynamicMaterial);
		}
	}
}

// Add instances at pose
void USLVizStaticMeshMarker::AddInstance(const FTransform& Pose)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
	AddInstanceChecked(Pose);
}

// Add instances with the poses
void USLVizStaticMeshMarker::AddInstances(const TArray<FTransform>& Poses)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
	AddInstancesChecked(Poses);
}

// Add instances with timeline update
void USLVizStaticMeshMarker::AddInstances(const TArray<FTransform>& Poses, float Duration, bool bLoop, float UpdateRate)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (Duration <= 0.008f)
	{
		AddInstancesChecked(Poses);
		return;
	}

	if (IsComponentTickEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d A timeline is already active, reset first.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Set the timeline data
	TimelinePoses = Poses;
	TimelineDuration = Duration;
	bLoopTimeline = bLoop;
	TimelineIndex = 0;

	// Start timeline
	if (UpdateRate > 0.f)
	{
		SetComponentTickInterval(UpdateRate);
	}
	SetComponentTickEnabled(true);
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizStaticMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
	ClearTimelineData();
}

// Unregister the component, remove it from its outer Actor's Components array and mark for pending kill
void USLVizStaticMeshMarker::DestroyComponent(bool bPromoteChildren)
{
	if (ISMC && ISMC->IsValidLowLevel() && !ISMC->IsPendingKillOrUnreachable())
	{
		ISMC->DestroyComponent();
	}
	Super::DestroyComponent(bPromoteChildren);
}

// Reset visual related data
void USLVizStaticMeshMarker::ResetVisuals()
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ISMC->EmptyOverrideMaterials();
}

// Reset instances (poses of the visuals)
void USLVizStaticMeshMarker::ResetPoses()
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ISMC->ClearInstances();
}
/* End VizMarker interface */

// Virtual add instance function
void USLVizStaticMeshMarker::AddInstanceChecked(const FTransform& Pose)
{
	ISMC->AddInstance(Pose);
}

// Virtual add instances function
void USLVizStaticMeshMarker::AddInstancesChecked(const TArray<FTransform>& Poses)
{
	for (const auto& P : Poses)
	{
		ISMC->AddInstance(P);
	}
}

// Reset the timeline related members
void USLVizStaticMeshMarker::ClearTimelineData()
{
	if (IsComponentTickEnabled())
	{
		SetComponentTickEnabled(false);
		SetComponentTickInterval(-1.f); // Tick every frame by default (If less than or equal to 0 then it will tick every frame)
	}
	TimelineIndex = INDEX_NONE;
	TimelinePoses.Empty();
}
