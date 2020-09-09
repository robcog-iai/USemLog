// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Marker/SLVizPrimitiveMarker.h"
#include "Viz/SLVizAssets.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizPrimitiveMarker::USLVizPrimitiveMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	ISMComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SLViz_ISM_Primitive"));
	ISMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	VisualScale = FVector(.1f);
	PrimitiveType = ESLVizPrimitiveMarkerType::NONE;
}

// Set the visual properties of the instanced mesh
void USLVizPrimitiveMarker::SetVisual(ESLVizPrimitiveMarkerType InType, float Size, const FLinearColor& InColor,  ESLVizMarkerMaterialType InMaterialType)
{
	// Clear any previous data
	Reset();

	// Set the visual mesh
	ISMComponent->SetStaticMesh(GetPrimitiveStaticMesh(InType));

	// Set the dynamic material
	SetDynamicMaterial(InMaterialType);
	SetDynamicMaterialColor(InColor);

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < ISMComponent->GetNumMaterials(); ++MatIdx)
	{
		ISMComponent->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Update the priomitive static mesh
void USLVizPrimitiveMarker::UpdatePrimitiveType(ESLVizPrimitiveMarkerType InType)
{
	ISMComponent->SetStaticMesh(GetPrimitiveStaticMesh(InType));

	// Apply dynamic material value
	for (int32 MatIdx = 0; MatIdx < ISMComponent->GetNumMaterials(); ++MatIdx)
	{
		ISMComponent->SetMaterial(MatIdx, DynamicMaterial);
	}
}

// Update the visual scale property
void USLVizPrimitiveMarker::UpdateSize(float Size)
{
	VisualScale = FVector(Size);

	ISMComponent->SetWorldScale3D(VisualScale);
	// or ?
	//for (const auto IB : ISMComponent->InstanceBodies)
	//{
	//	IB->Scale3D = VisualScale;
	//}
}

// Add instances at pose
void USLVizPrimitiveMarker::AddInstance(const FTransform& Pose)
{
	ISMComponent->AddInstance(FTransform(Pose.GetRotation(), Pose.GetLocation(), VisualScale));
}

// Add instances with the poses
void USLVizPrimitiveMarker::AddInstances(const TArray<FTransform>& Poses)
{
	for (auto P : Poses)
	{
		P.SetScale3D(VisualScale);
		ISMComponent->AddInstance(P);
	}
}

/* Begin VizMarker interface */
// Reset visuals and poses
void USLVizPrimitiveMarker::Reset()
{
	ResetVisuals();
	ResetPoses();
}

// Reset visual related data
void USLVizPrimitiveMarker::ResetVisuals()
{
	ISMComponent->EmptyOverrideMaterials();
}

// Reset instances (poses of the visuals)
void USLVizPrimitiveMarker::ResetPoses()
{
	ISMComponent->ClearInstances();
}
/* End VizMarker interface */


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
	case ESLVizPrimitiveMarkerType::Axis:
		return VizAssetsContainer->MeshAxis;
	default:
		UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown primitve type, defaulted to box.."), *FString(__FUNCTION__), __LINE__);
		return VizAssetsContainer->MeshBox;
	}
}
