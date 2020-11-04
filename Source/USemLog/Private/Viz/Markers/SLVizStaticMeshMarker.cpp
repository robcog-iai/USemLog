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
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLVizStaticMeshMarker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Increase the passed time
	TimelineDeltaTime += DeltaTime;

	// Calculate the number of instances to draw depending on the passed time
	int32 NumInstancesToDraw = (TimelineDeltaTime * TimelinePoses.Num()) / TimelineDuration;

	// Wait if not enough time has passed
	if (NumInstancesToDraw == 0)
	{
		return;
	}	

	// Draw all instances, or use a limit
	if (TimelineMaxNumInstances <= 0)
	{
		UpdateTimeline(NumInstancesToDraw);
	}
	else
	{
		UpdateTimelineWithMaxNumInstances(NumInstancesToDraw);
	}

	// Reset the elapsed time
	TimelineDeltaTime = 0;
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
void USLVizStaticMeshMarker::AddInstances(const TArray<FTransform>& Poses,
	const FSLVizTimelineParams& TimelineParams)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (TimelineParams.Duration <= 0.008f)
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
	TimelineDuration = TimelineParams.Duration;
	TimelineMaxNumInstances = TimelineParams.MaxNumInstances;
	bLoopTimeline = TimelineParams.bLoop;

	TimelineIndex = 0;
	
	// Start timeline
	if (TimelineParams.UpdateRate > 0.f)
	{
		SetComponentTickInterval(TimelineParams.UpdateRate);
	}
	SetComponentTickEnabled(true);
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizStaticMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
	ClearAndStopTimeline();
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

// Virtual update instance transform
bool USLVizStaticMeshMarker::UpdateInstanceTransform(int32 Index, const FTransform& Pose)
{
	return ISMC->UpdateInstanceTransform(Index, Pose, true, true, true);
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
void USLVizStaticMeshMarker::ClearAndStopTimeline()
{
	if (IsComponentTickEnabled())
	{
		SetComponentTickEnabled(false);
		SetComponentTickInterval(-1.f); // Tick every frame by default (If less than or equal to 0 then it will tick every frame)
	}
	TimelineMaxNumInstances = INDEX_NONE;
	TimelineIndex = INDEX_NONE;
	TimelinePoses.Empty();
}

// Update timeline with the given number of new instances
void USLVizStaticMeshMarker::UpdateTimeline(int32 NumNewInstances)
{
	// Check if it is safe to draw all new instances
	if (TimelineIndex + NumNewInstances < TimelinePoses.Num())
	{
		// Safe to draw all new instances
		while (NumNewInstances > 0)
		{
			AddInstanceChecked(TimelinePoses[TimelineIndex]);
			TimelineIndex++;
			NumNewInstances--;
		}
	}
	else
	{
		// Reached end of the poses array, add only remaining values
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
			ClearAndStopTimeline();
		}
	}
}

// Update timeline with max number of instances
void USLVizStaticMeshMarker::UpdateTimelineWithMaxNumInstances(int32 NumNewInstances)
{
	// Check if it is safe to draw all new instances
	if (TimelineIndex + NumNewInstances < TimelinePoses.Num())
	{
		// Safe to draw all new instances
		while (NumNewInstances > 0)
		{
			if (TimelineIndex < TimelineMaxNumInstances)
			{
				AddInstanceChecked(TimelinePoses[TimelineIndex]);
			}
			else
			{
				int32 UpdateIndex = TimelineIndex % TimelineMaxNumInstances;
				UpdateInstanceTransform(UpdateIndex, TimelinePoses[TimelineIndex]);
			}
			TimelineIndex++;
			NumNewInstances--;
		}
	}
	else
	{
		// Reached end of the poses array, add only remaining values
		while (TimelinePoses.IsValidIndex(TimelineIndex))
		{
			if (TimelineIndex < TimelineMaxNumInstances)
			{
				AddInstanceChecked(TimelinePoses[TimelineIndex]);
			}
			else
			{
				int32 UpdateIndex = TimelineIndex % TimelineMaxNumInstances;
				UpdateInstanceTransform(UpdateIndex, TimelinePoses[TimelineIndex]);
			}
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
			ClearAndStopTimeline();
		}
	}
}
