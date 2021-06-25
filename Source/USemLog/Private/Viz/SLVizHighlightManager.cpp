// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizHighlightManager.h"
#include "Viz/SLVizAssets.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_EDITOR
#include "Editor.h" // for Editor Delegates
#endif

// Sets default values for this component's properties
ASLVizHighlightManager::ASLVizHighlightManager()
{
	PrimaryActorTick.bCanEverTick = false;

	LoadAssetsContainer();

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Do any object-specific cleanup required immediately after loading an object. (called only once when loading the map)
void ASLVizHighlightManager::PostLoad()
{
	Super::PostLoad();
	BindDelgates();
}

// When an actor is created in the editor or during gameplay, this gets called right before construction. This is not called for components loaded from a level. 
void ASLVizHighlightManager::PostActorCreated()
{
	Super::PostActorCreated();
	BindDelgates();
}

// Called when actor removed from game or game ended
void ASLVizHighlightManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreOriginalMaterials();
	Super::EndPlay(EndPlayReason);
}

// Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending 
void ASLVizHighlightManager::Destroyed()
{
	RestoreOriginalMaterials();
	Super::Destroyed();
}

// Load container with the vizual assets
bool ASLVizHighlightManager::LoadAssetsContainer()
{
	static ConstructorHelpers::FObjectFinder<USLVizAssets>VizAssetsContainerAsset(AssetsContainerPath);
	if (VizAssetsContainerAsset.Succeeded())
	{
		VizAssetsContainer = VizAssetsContainerAsset.Object;

		// Check if all assets in the container are set
		bool RetVal = true;

		/* Materials */
		if (VizAssetsContainer->MaterialHighlightAdditive == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialHighlightAdditive is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MaterialHighlightTranslucent == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialHighlightTranslucent is NULL.."), *FString(__FUNCTION__), __LINE__);
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

// Callback function registered with global world delegates to reset materials to their original values
void ASLVizHighlightManager::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	RestoreOriginalMaterials();
}

// Make sure the original materials are applied if the manager is destroyed or the level closed etc.
void ASLVizHighlightManager::RestoreOriginalMaterials()
{
	ClearAllHighlights();
	RemoveDelegates();
}


// Highlight a static mesh
void ASLVizHighlightManager::Highlight(UMeshComponent* MC, const FSLVizVisualParams& VisualParams)
{
	// Cache the original materials
	HighlightedStaticMeshes.Add(MC, FSLVizHighlightData(MC->GetMaterials(), VisualParams.MaterialSlots));

	UMaterialInstanceDynamic* DynMat = CreateTransientMID(VisualParams.MaterialType);
	DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);
	
	if (VisualParams.MaterialSlots.Num() > 0)
	{
		for (int32 MatIdx : VisualParams.MaterialSlots)
		{
			MC->SetMaterial(MatIdx, DynMat);
		}
	}
	else
	{
		for (int32 MatIdx = 0; MatIdx < MC->GetNumMaterials(); ++MatIdx)
		{
			MC->SetMaterial(MatIdx, DynMat);
		}
	}
}

// Update the visual of the given mesh component
void ASLVizHighlightManager::UpdateHighlight(UMeshComponent* MC, const FSLVizVisualParams& VisualParams)
{
	if (auto HighlightData = HighlightedStaticMeshes.Find(MC))
	{
		UMaterialInstanceDynamic* DynMat = CreateTransientMID(VisualParams.MaterialType);
		DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);

		if (HighlightData->MaterialSlots.Num() > 0)
		{
			for (int32 MatIdx : HighlightData->MaterialSlots)
			{
				MC->SetMaterial(MatIdx, DynMat);
			}
		}
		else
		{
			for (int32 MatIdx = 0; MatIdx < MC->GetNumMaterials(); ++MatIdx)
			{
				MC->SetMaterial(MatIdx, DynMat);
			}
		}
	}
}

// Clear highlight of a static mesh
void ASLVizHighlightManager::ClearHighlight(UMeshComponent* MC)
{
	FSLVizHighlightData HighlightData;
	if (HighlightedStaticMeshes.RemoveAndCopyValue(MC, HighlightData))
	{
		if (HighlightData.MaterialSlots.Num() > 0)
		{
			for (int32 MatIdx : HighlightData.MaterialSlots)
			{
				MC->SetMaterial(MatIdx, HighlightData.OriginalMaterials[MatIdx]);
			}
		}
		else
		{
			for (int32 MatIdx = 0; MatIdx < HighlightData.OriginalMaterials.Num(); ++MatIdx)
			{
				MC->SetMaterial(MatIdx, HighlightData.OriginalMaterials[MatIdx]);
			}
		}
	}
}

// Clear all highlights
void ASLVizHighlightManager::ClearAllHighlights()
{
	for (const auto& SMToMatsPair : HighlightedStaticMeshes)
	{
		for (int32 MatIdx = 0; MatIdx < SMToMatsPair.Value.OriginalMaterials.Num(); ++MatIdx)
		{
			SMToMatsPair.Key->SetMaterial(MatIdx, SMToMatsPair.Value.OriginalMaterials[MatIdx]);
		}
	}
	HighlightedStaticMeshes.Empty();
}

// Bind delegates
void ASLVizHighlightManager::BindDelgates()
{
	if (!FWorldDelegates::OnWorldCleanup.IsBoundToObject(this))
	{
		FWorldDelegates::OnWorldCleanup.AddUObject(this, &ASLVizHighlightManager::OnWorldCleanup);
	}
#if WITH_EDITOR
	if (!GEditor->OnEditorClose().IsBoundToObject(this))
	{
		GEditor->OnEditorClose().AddUObject(this, &ASLVizHighlightManager::RestoreOriginalMaterials);
	}
#endif
}

// Remove any bound delegates
void ASLVizHighlightManager::RemoveDelegates()
{
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
#if WITH_EDITOR
	GEditor->OnEditorClose().RemoveAll(this);
#endif
}


// Create a dynamic material instance
UMaterialInstanceDynamic* ASLVizHighlightManager::CreateTransientMID(ESLVizMaterialType InMaterialType)
{
	switch (InMaterialType)
	{
	case(ESLVizMaterialType::Additive):
		return UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, nullptr);
	case(ESLVizMaterialType::Translucent):
		return UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightTranslucent, nullptr);
	case(ESLVizMaterialType::Lit):
		return UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialLit, nullptr);
	case(ESLVizMaterialType::Unlit):
		return UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialUnlit, nullptr);
	default:
		return UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, nullptr);
	}
	return nullptr;
}