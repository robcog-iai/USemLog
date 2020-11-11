// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Markers/SLVizBaseMarker.h"
#include "Viz/SLVizAssets.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizBaseMarker::USLVizBaseMarker()
{
	PrimaryComponentTick.bCanEverTick = false;

	LoadAssetsContainer();

	MaterialType = ESLVizMaterialType::NONE;
	VisualColor = FLinearColor::Transparent; // (0,0,0,0)

	TimelineIndex = INDEX_NONE;
	TimelineMaxNumInstances = INDEX_NONE;
	bLoopTimeline = false;
}

// Update the visual color property
void USLVizBaseMarker::UpdateMaterialColor(const FLinearColor& InColor)
{
	SetDynamicMaterialColor(InColor);
}

// Update the material type
void USLVizBaseMarker::UpdateMaterialType(ESLVizMaterialType InType)
{
	SetDynamicMaterial(InType);
}

// Unregister the component, remove it from its outer Actor's Components array and mark for pending kill
void USLVizBaseMarker::DestroyComponent(bool bPromoteChildren)
{
	if (DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		DynamicMaterial->ConditionalBeginDestroy();
	}
	Super::DestroyComponent(bPromoteChildren);
}

// Create the dynamic material
void USLVizBaseMarker::SetDynamicMaterial(ESLVizMaterialType InType)
{
	if (DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		if (InType == MaterialType)
		{
			// Creating a material of the same type, skip
			return;
		}
		else
		{
			// Destroy previous material
			DynamicMaterial->ConditionalBeginDestroy();
		}
	}
	
	MaterialType = InType;

	// Create new material
	switch (InType)
	{
	case(ESLVizMaterialType::Unlit):
		DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialUnlit, this);
		break;
	case(ESLVizMaterialType::Lit):
		DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialLit, this);
		break;
	case(ESLVizMaterialType::Additive):
		DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, this);
		break;
	case(ESLVizMaterialType::Translucent):
		DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightTranslucent, this);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Unknown material type, set as lit.."), *FString(__FUNCTION__), __LINE__);
		DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialLit, this);
		MaterialType = ESLVizMaterialType::Lit;
	}
}

// Set the color of the dynamic material
void USLVizBaseMarker::SetDynamicMaterialColor(const FLinearColor& InColor)
{
	if (DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		if (!InColor.Equals(VisualColor))
		{
			DynamicMaterial->SetVectorParameterValue(FName("Color"), InColor);
			VisualColor = InColor;
		}
	}
}

// Load assets container
bool USLVizBaseMarker::LoadAssetsContainer()
{
	static ConstructorHelpers::FObjectFinder<USLVizAssets>VizAssetsContainerAsset(AssetsContainerPath);
	if (VizAssetsContainerAsset.Succeeded())
	{
		VizAssetsContainer = VizAssetsContainerAsset.Object;

		// Check if all assets in the container are set
		bool RetVal = true;

		/* Meshes */
		if (VizAssetsContainer->MeshBox == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshBox is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshSphere == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshSphere is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshCylinder == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshCylinder is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshArrow == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshArrow is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshAxis == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshAxis is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}

		/* Materials */
		if (VizAssetsContainer->MaterialLit == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialLit is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MaterialUnlit == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialUnlit is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MaterialInvisible == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialInvisible is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}

		return RetVal;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the assets container at Path=%s.."),
			*FString(__FUNCTION__), __LINE__, AssetsContainerPath);
		return false;
	}
}
