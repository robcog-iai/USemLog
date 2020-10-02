// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Legacy/SLVizHighlightMarker.h"
#include "Viz/Legacy/SLVizHighlightMarkerManager.h"
#include "Viz/SLVizAssets.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizHighlightMarker::USLVizHighlightMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	LoadAssetsContainer();
	HighlightSMC = nullptr;
	HighlightSkelMC = nullptr;
	DynamicMaterial = nullptr;
	MaterialType = ESLVizHighlightMarkerMaterialType::NONE;
}

// Highlight the given static mesh by creating a clone
void USLVizHighlightMarker::Set(UStaticMeshComponent* SMC, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	// (If previously set) destroy skeletal component
	ClearSkeletalMeshComponent();

	if (SetStaticMeshComponent(SMC))
	{
		SetDynamicMaterial(VisualParams);
		for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSMC->SetMaterial(MatIdx, DynamicMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not set the static mesh.."), *FString(__FUNCTION__), __LINE__);
	}

	// Make sure highlight marker moves with its representation
	AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given skeletal mesh by creating a clone
void USLVizHighlightMarker::Set(USkeletalMeshComponent* SkMC, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	// (If previously set) destroy static mesh component
	ClearStaticMeshComponent();

	// Clear materials, or create new component
	if (SetSkeletalMeshComponent(SkMC))
	{
		TArray<FName> BoneNames;
		SkMC->GetBoneNames(BoneNames);
		for (const auto Name : BoneNames)
		{
			HighlightSkelMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
		}

		SetDynamicMaterial(VisualParams);
		for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial);
		}

		// All material slots used
		SkeletalMaterialIndexes.Empty();
	}

	// Make sure highlight marker moves with its representation
	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bone (material index) skeletal mesh by creating a clone
void USLVizHighlightMarker::Set(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	// (If previously set) destroy static mesh component
	ClearStaticMeshComponent();

	if (MaterialIndex >= SkMC->GetNumMaterials())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MaterialIndex);
		return;
	}

	// Clear materials, or create new component
	if (SetSkeletalMeshComponent(SkMC))
	{
		//HighlightSkelMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
		//HighlightSkelMC->MarkRefreshTransformDirty();

		//for (int32 BoneIdx = 0; BoneIdx < SkMC->GetNumBones(); ++BoneIdx)
		//{
		//	HighlightSkelMC->BoneSpaceTransforms[BoneIdx] = SkMC->BoneSpaceTransforms[BoneIdx];
		//}

		TArray<FName> BoneNames;
		SkMC->GetBoneNames(BoneNames);
		for (const auto Name : BoneNames)
		{
			HighlightSkelMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
		}

		SetDynamicMaterial(VisualParams);
		for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
		{
			MatIdx == MaterialIndex ? HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial) :
				HighlightSkelMC->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible);
		}

		// One material slot used
		SkeletalMaterialIndexes.Empty();
		SkeletalMaterialIndexes.Add(MaterialIndex);
	}

	// Make sure highlight marker moves with its representation
	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bones (material indexes) skeletal mesh by creating a clone
void USLVizHighlightMarker::Set(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	// (If previously set) destroy static mesh component
	ClearStaticMeshComponent();

	// Clear materials, or create new component
	if (SetSkeletalMeshComponent(SkMC))
	{
		//HighlightSkelMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
		//HighlightSkelMC->MarkRefreshTransformDirty();

		//for (int32 BoneIdx = 0; BoneIdx < SkMC->GetNumBones(); ++BoneIdx)
		//{
		//	HighlightSkelMC->BoneSpaceTransforms[BoneIdx] = SkMC->BoneSpaceTransforms[BoneIdx];
		//}

		TArray<FName> BoneNames;
		SkMC->GetBoneNames(BoneNames);
		for (const auto Name : BoneNames)
		{
			HighlightSkelMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
		}

		SetDynamicMaterial(VisualParams);
		for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
		{
			MaterialIndexes.Contains(MatIdx) ? HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial)
				: HighlightSkelMC->SetMaterial(MatIdx, VizAssetsContainer->MaterialInvisible);
		}

		// Mulitple material slot used
		SkeletalMaterialIndexes.Empty();
		SkeletalMaterialIndexes = MaterialIndexes;
	}

	// Make sure highlight marker moves with its representation
	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Set the visual parameters
bool USLVizHighlightMarker::UpdateVisualParameters(const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	// Check visual mesh type
	if (HighlightSMC && HighlightSMC->IsValidLowLevel() && !HighlightSMC->IsPendingKillOrUnreachable())
	{
		SetDynamicMaterial(VisualParams);
		for (int32 MatIdx = 0; MatIdx < HighlightSMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSMC->SetMaterial(MatIdx, DynamicMaterial);
		}
		return true;
	}
	else if (HighlightSkelMC && HighlightSkelMC->IsValidLowLevel() && !HighlightSkelMC->IsPendingKillOrUnreachable())
	{
		if (SkeletalMaterialIndexes.Num() == 0)
		{
			SetDynamicMaterial(VisualParams);
			for (int32 MatIdx = 0; MatIdx < HighlightSkelMC->GetNumMaterials(); ++MatIdx)
			{
				HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial);
			}
			return true;
		}
		else if (SkeletalMaterialIndexes.Num() == 1)
		{
			SetDynamicMaterial(VisualParams);
			HighlightSkelMC->SetMaterial(SkeletalMaterialIndexes[0], DynamicMaterial);
			return true;
		}
		else
		{
			SetDynamicMaterial(VisualParams);
			for (int32 MatIdx : SkeletalMaterialIndexes)
			{
				if (MatIdx >= HighlightSkelMC->GetNumMaterials())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MatIdx);
					continue;
				}
				HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial);
			}
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Highlight marker does not have a valid visual mesh set.."), *FString(__FUNCTION__), __LINE__);
	}
	return false;
}

// Clear marker by notifing parent manager
bool USLVizHighlightMarker::DestroyThroughManager()
{
	if (ASLVizHighlightMarkerManager* Manager = Cast<ASLVizHighlightMarkerManager>(GetOwner()))
	{
		Manager->ClearMarker(this);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access manager.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
}

// Destroy dynamically created components first
void USLVizHighlightMarker::DestroyComponent(bool bPromoteChildren/*= false*/)
{
	ClearDynamicMaterial();
	ClearStaticMeshComponent();
	ClearSkeletalMeshComponent();

	Super::DestroyComponent(bPromoteChildren);
}

// Load assets container
bool USLVizHighlightMarker::LoadAssetsContainer()
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

// Set the static mesh component
bool USLVizHighlightMarker::SetStaticMeshComponent(UStaticMeshComponent* SMC)
{
	// Clear materials, or create new component
	if (HighlightSMC && HighlightSMC->IsValidLowLevel() && !HighlightSMC->IsPendingKillOrUnreachable())
	{
		HighlightSMC->EmptyOverrideMaterials();
	}
	else
	{
		HighlightSMC = NewObject<UStaticMeshComponent>(this);
		HighlightSMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSMC->RegisterComponent();
	}

	// Set the mesh visual
	if (HighlightSMC->SetStaticMesh(SMC->GetStaticMesh()))
	{
		HighlightSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightSMC->bSelectable = false;
		return true;
	}
	return false;
}

// Clear the static mesh component
void USLVizHighlightMarker::ClearStaticMeshComponent()
{
	if (HighlightSMC && HighlightSMC->IsValidLowLevel() && !HighlightSMC->IsPendingKillOrUnreachable())
	{
		//HighlightSMC->ConditionalBeginDestroy();
		HighlightSMC->DestroyComponent();
	}
	HighlightSMC = nullptr;
}

// Set the skeletal mesh component
bool USLVizHighlightMarker::SetSkeletalMeshComponent(USkeletalMeshComponent* SkMC)
{
	// Clear materials, or create new component
	if (HighlightSkelMC && HighlightSkelMC->IsValidLowLevel() && !HighlightSkelMC->IsPendingKillOrUnreachable())
	{
		HighlightSkelMC->EmptyOverrideMaterials();
	}
	else
	{
		HighlightSkelMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkelMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSkelMC->RegisterComponent();
	}

	// Set the mesh visual
	HighlightSkelMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	HighlightSkelMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightSkelMC->bSelectable = false;

	return HighlightSkelMC && HighlightSkelMC->IsValidLowLevel() && !HighlightSkelMC->IsPendingKillOrUnreachable();
}

// Clear the skeletal mesh component
void USLVizHighlightMarker::ClearSkeletalMeshComponent()
{
	if (HighlightSkelMC && HighlightSkelMC->IsValidLowLevel() && !HighlightSkelMC->IsPendingKillOrUnreachable())
	{
		//HighlightSkelMC->ConditionalBeginDestroy();
		HighlightSkelMC->DestroyComponent();
	}
	HighlightSkelMC = nullptr;
	SkeletalMaterialIndexes.Empty();
}

// Set the dynamic material
void USLVizHighlightMarker::SetDynamicMaterial(const FSLVizHighlightMarkerVisualParams& VisualParams)
{
	const bool bIsMaterialValid = DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable();
	if (bIsMaterialValid)
	{
		const bool bIsValidMaterialOfTheSameType = VisualParams.MaterialType != MaterialType;
		if (!bIsValidMaterialOfTheSameType)
		{
			// Destroy previous material and create a new one
			DynamicMaterial->ConditionalBeginDestroy();
			if (VisualParams.MaterialType == ESLVizHighlightMarkerMaterialType::Additive)
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);
			}
			else if (VisualParams.MaterialType == ESLVizHighlightMarkerMaterialType::Translucent)
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightTranslucent, NULL);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Unknown material type, using default as translucent.. "), *FString(__FUNCTION__), __LINE__);
				DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);
			}
		}
	}
	else
	{
		// Create dynamic material
		if (VisualParams.MaterialType == ESLVizHighlightMarkerMaterialType::Additive)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);
		}
		else if (VisualParams.MaterialType == ESLVizHighlightMarkerMaterialType::Translucent)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightTranslucent, NULL);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Unknown material type, using default as translucent.. "), *FString(__FUNCTION__), __LINE__);
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);
		}
	}

	DynamicMaterial->SetVectorParameterValue(FName("Color"), VisualParams.Color);
	MaterialType = VisualParams.MaterialType;
}

// Clear the dynamic material
void USLVizHighlightMarker::ClearDynamicMaterial()
{
	if (DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		DynamicMaterial->ConditionalBeginDestroy();
	}
	DynamicMaterial = nullptr;
}
