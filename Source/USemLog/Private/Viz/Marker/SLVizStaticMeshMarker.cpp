// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Marker/SLVizStaticMeshMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

// Constructor
USLVizStaticMeshMarker::USLVizStaticMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	ISMC = nullptr;	
	TimelineIndex = INDEX_NONE;
	bLoopTimeline = false;
}

// Called every frame, used for timeline visualizations, activated and deactivated on request
void USLVizStaticMeshMarker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UE_LOG(LogTemp, Log, TEXT("%s::%d DeltaTime = %f"), *FString(__FUNCTION__), __LINE__, DeltaTime);
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

	for (const auto& P : Poses)
	{
		AddInstanceChecked(P);
	}
}

// Add instances with timeline update
void USLVizStaticMeshMarker::AddInstances(const TArray<FTransform>& Poses, float Duration, bool bLoop, float UpdateRate)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Check if there is already an active timeline
	if (IsComponentTickEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d A timeline is already active, reset first.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (UpdateRate > 0.f)
	{
		SetComponentTickInterval(UpdateRate);
	}
	SetComponentTickEnabled(true);

	RegisterComponentTickFunctions(true); // TODO chek this
}

// Add instances with the poses using and update rate
void USLVizStaticMeshMarker::AddTimeline(const TArray<FTransform>& Poses, float UpdateRate, bool bLoop, float StartDelay)
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Check if there is already an active timeline
	if (GetWorld()->GetTimerManager().IsTimerActive(TimelineTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Timeline is already active.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Make sure there are enough poses in the timeline
	if (Poses.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d The timeline should have at least 2 poses.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Set the timeline pose data
	TimelinePoses = Poses;

	// Should the timeline animation be repeated
	bLoopTimeline = bLoop;

	// Set index position to first
	TimelineIndex = 0;

	if (UpdateRate <= 0.f)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d UpdateRate cannot be smaller than 0, default value used.."),
			*FString(__FUNCTION__), __LINE__);
		UpdateRate = 0.08333f; // ~120fps
	}

	// Bind and start timer callback
	GetWorld()->GetTimerManager().SetTimer(TimelineTimerHandle, this, &USLVizStaticMeshMarker::TimelineUpdateCallback,
		UpdateRate, true, StartDelay);

	SetComponentTickEnabled(true);
	SetComponentTickInterval(UpdateRate);
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizStaticMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
	ResetTimeline();
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


// Reset the timeline related members
void USLVizStaticMeshMarker::ResetTimeline()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TimelineTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimelineTimerHandle);
	}
	TimelineIndex = INDEX_NONE;
	TimelinePoses.Empty();
}

// Used as an timer update callback when the instance should be added as a timeline
void USLVizStaticMeshMarker::TimelineUpdateCallback()
{
	if (TimelinePoses.IsValidIndex(TimelineIndex))
	{
		AddInstanceChecked(TimelinePoses[TimelineIndex]);
		TimelineIndex++;
	}
	else if(bLoopTimeline)
	{
		ISMC->ClearInstances();
		TimelineIndex = 0;
		SetComponentTickEnabled(false);
	}
	else
	{
		ResetTimeline();
	}
}
