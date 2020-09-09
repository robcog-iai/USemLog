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
	
	ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SLViz_ISM_StaticMesh"));
	ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Set the visual properties of the instanced mesh
void USLVizStaticMeshMarker::SetVisual(UStaticMeshComponent* SMC, const FLinearColor& InColor,	ESLVizMarkerMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	// Set the visual mesh
	ISMComponent->SetStaticMesh(SMC->GetStaticMesh());

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < ISMComponent->GetNumMaterials(); ++MatIdx)
	{
		ISMComponent->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Update the visual mesh type
void USLVizStaticMeshMarker::UpdateStaticMesh(UStaticMeshComponent* SMC)
{
	ISMComponent->SetStaticMesh(SMC->GetStaticMesh());

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < ISMComponent->GetNumMaterials(); ++MatIdx)
	{
		ISMComponent->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Add instances at pose
void USLVizStaticMeshMarker::AddInstance(const FTransform& Pose)
{
	ISMComponent->AddInstance(Pose);
}

// Add instances with the poses
void USLVizStaticMeshMarker::AddInstances(const TArray<FTransform>& Poses)
{
	for (const auto& P : Poses)
	{
		ISMComponent->AddInstance(P);
	}
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizStaticMeshMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
}

// Reset visual related data
void USLVizStaticMeshMarker::ResetVisuals()
{
	ISMComponent->EmptyOverrideMaterials();
}

// Reset instances (poses of the visuals)
void USLVizStaticMeshMarker::ResetPoses()
{
	ISMComponent->ClearInstances();
}
/* End VizMarker interface */
