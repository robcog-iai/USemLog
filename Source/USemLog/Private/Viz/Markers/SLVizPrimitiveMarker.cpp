// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Markers/SLVizPrimitiveMarker.h"
#include "Viz/SLVizAssets.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizPrimitiveMarker::USLVizPrimitiveMarker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	MarkerScale = FVector(.1f);
	PrimitiveType = ESLVizPrimitiveMarkerType::NONE;
}

// Set the visual properties of the instanced mesh
void USLVizPrimitiveMarker::SetVisual(ESLVizPrimitiveMarkerType InType, float Size, const FLinearColor& InColor,  ESLVizMaterialType InMaterialType)
{
	// Set scale
	MarkerScale = FVector(Size);
	// Inheritance hides functions with the same name (even for if with different parameters)
	USLVizStaticMeshMarker::SetVisual(GetPrimitiveStaticMesh(InType), InColor, InMaterialType);
}

// Update the priomitive static mesh
void USLVizPrimitiveMarker::UpdatePrimitiveType(ESLVizPrimitiveMarkerType InType)
{
	UpdateStaticMesh(GetPrimitiveStaticMesh(InType));
}

// Update the visual scale property
void USLVizPrimitiveMarker::UpdateSize(float Size)
{
	if (!ISMC && !ISMC->IsValidLowLevel() && ISMC->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual is not set.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	MarkerScale = FVector(Size);
	for (const auto IB : ISMC->InstanceBodies)
	{
		IB->Scale3D = MarkerScale;
	}
}

// Virtual add instance function
void USLVizPrimitiveMarker::AddInstanceChecked(const FTransform& Pose)
{
	ISMC->AddInstance(FTransform(Pose.GetRotation(), Pose.GetLocation(), MarkerScale));
}

// Virtual update instance transform
bool USLVizPrimitiveMarker::UpdateInstanceTransform(int32 Index, const FTransform& Pose)
{
	return ISMC->UpdateInstanceTransform(Index, FTransform(Pose.GetRotation(), Pose.GetLocation(), MarkerScale), true, true, true);
}

// Virtual add instances function
void USLVizPrimitiveMarker::AddInstancesChecked(const TArray<FTransform>& Poses)
{
	for (auto P : Poses)
	{
		P.SetScale3D(MarkerScale);
		ISMC->AddInstance(P);
	}
}

// Get the static mesh of the primitive type
UStaticMesh* USLVizPrimitiveMarker::GetPrimitiveStaticMesh(ESLVizPrimitiveMarkerType InType) const
{
	switch (InType)
	{
	case ESLVizPrimitiveMarkerType::Box:
		return VizAssetsContainer->MeshBox;
	case ESLVizPrimitiveMarkerType::Sphere:
		return VizAssetsContainer->MeshSphere;
	case ESLVizPrimitiveMarkerType::Cylinder:
		return VizAssetsContainer->MeshCylinder;
	case ESLVizPrimitiveMarkerType::Arrow:
		return VizAssetsContainer->MeshArrow;
	case ESLVizPrimitiveMarkerType::ArrowX:
		return VizAssetsContainer->MeshArrowX;
	case ESLVizPrimitiveMarkerType::ArrowY:
		return VizAssetsContainer->MeshArrowY;
	case ESLVizPrimitiveMarkerType::ArrowZ:
		return VizAssetsContainer->MeshArrowZ;
	case ESLVizPrimitiveMarkerType::Axis:
		return VizAssetsContainer->MeshAxis;
	default:
		UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown primitve type, defaulted to box.."), *FString(__FUNCTION__), __LINE__);
		return VizAssetsContainer->MeshBox;
	}
}
