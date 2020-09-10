// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Marker/SLVizStaticMeshMarker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizStaticMeshMarker::USLVizStaticMeshMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	ISMC = nullptr;	
}

// Set the visual properties of the instanced mesh
void USLVizStaticMeshMarker::SetVisual(UStaticMesh* SM, const FLinearColor& InColor, ESLVizMarkerMaterialType InMaterialType)
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
	for (int32 MatIdx = 0; MatIdx < ISMC->GetNumMaterials(); ++MatIdx)
	{
		ISMC->SetMaterial(MatIdx, DynamicMaterial);
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

	ISMC->AddInstance(Pose);
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
		ISMC->AddInstance(P);
	}
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizStaticMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ISMC->EmptyOverrideMaterials();
}

// Reset instances (poses of the visuals)
void USLVizStaticMeshMarker::ResetPoses()
{
	if (!ISMC || !ISMC->IsValidLowLevel() || ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ISMC->ClearInstances();
}
/* End VizMarker interface */
