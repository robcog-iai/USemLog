// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizHighlightMarker.h"
#include "Viz/SLVizHighlightMarkerManager.h"
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
	HighlightSkMC = nullptr;
	DynamicMaterial = nullptr;
	MaterialType = ESLVizHighlightMarkerType::NONE;
}

// Highlight the given static mesh by creating a clone
void USLVizHighlightMarker::Init(UStaticMeshComponent* SMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (!HighlightSMC)
	{
		HighlightSMC = NewObject<UStaticMeshComponent>(this);
		HighlightSMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSMC->RegisterComponent();
	}
	else
	{
		HighlightSMC->EmptyOverrideMaterials();
	}

	if (HighlightSkMC)
	{
		if (HighlightSkMC->IsValidLowLevel())
		{
			//HighlightSkMC->DestroyComponent();
			HighlightSkMC->ConditionalBeginDestroy();
		}
	}

	if (HighlightSMC->SetStaticMesh(SMC->GetStaticMesh()))
	{
		SetWorldTransform(SMC->GetComponentTransform());
		HighlightSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetDynamicMaterial(Color, Type);
		for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSMC->SetMaterial(MatIdx, DynamicMaterial);
		}
	}

	AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (!HighlightSkMC)
	{
		HighlightSkMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSkMC->RegisterComponent();
	}
	else
	{
		HighlightSkMC->EmptyOverrideMaterials();
	}

	if (HighlightSMC)
	{
		if (HighlightSMC->IsValidLowLevel())
		{
			//HighlightSMC->DestroyComponent();
			HighlightSMC->ConditionalBeginDestroy();
		}
	}

	HighlightSkMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	SetWorldTransform(SkMC->GetComponentTransform());

	//HighlightSkMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
	//HighlightSkMC->MarkRefreshTransformDirty();

	TArray<FName> BoneNames;
	SkMC->GetBoneNames(BoneNames);
	for (const auto Name : BoneNames)
	{
		HighlightSkMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
	}

	SetDynamicMaterial(Color, Type);
	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		HighlightSkMC->SetMaterial(MatIdx, DynamicMaterial);
	}


	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bone (material index) skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (MaterialIndex >= SkMC->GetNumMaterials())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MaterialIndex);
		return;
	}

	if (!HighlightSkMC)
	{
		HighlightSkMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSkMC->RegisterComponent();
	}
	else
	{
		HighlightSkMC->EmptyOverrideMaterials();
	}

	if (HighlightSMC)
	{
		//HighlightSMC->DestroyComponent();
		HighlightSMC->ConditionalBeginDestroy();
	}

	HighlightSkMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	SetWorldTransform(SkMC->GetComponentTransform());

	//HighlightSkMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
	//HighlightSkMC->MarkRefreshTransformDirty();

	//for (int32 BoneIdx = 0; BoneIdx < SkMC->GetNumBones(); ++BoneIdx)
	//{
	//	HighlightSkMC->BoneSpaceTransforms[BoneIdx] = SkMC->BoneSpaceTransforms[BoneIdx];
	//}

	TArray<FName> BoneNames;
	SkMC->GetBoneNames(BoneNames);
	for (const auto Name : BoneNames)
	{
		HighlightSkMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
	}

	SetDynamicMaterial(Color, Type);
	HighlightSkMC->SetMaterial(MaterialIndex, DynamicMaterial);

	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bones (material indexes) skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (!HighlightSkMC)
	{
		HighlightSkMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkMC->RegisterComponent();
		HighlightSkMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		HighlightSkMC->EmptyOverrideMaterials();
	}

	if (HighlightSMC)
	{
		if (HighlightSMC->IsValidLowLevel())
		{
			//HighlightSMC->DestroyComponent();
			HighlightSMC->ConditionalBeginDestroy();
		}
	}

	HighlightSkMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	SetWorldTransform(SkMC->GetComponentTransform());

	//HighlightSkMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
	//HighlightSkMC->MarkRefreshTransformDirty();

	//for (int32 BoneIdx = 0; BoneIdx < SkMC->GetNumBones(); ++BoneIdx)
	//{
	//	HighlightSkMC->BoneSpaceTransforms[BoneIdx] = SkMC->BoneSpaceTransforms[BoneIdx];
	//}

	TArray<FName> BoneNames;
	SkMC->GetBoneNames(BoneNames);
	for (const auto Name : BoneNames)
	{
		HighlightSkMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
	}

	SetDynamicMaterial(Color, Type);
	for (int32 MatIdx : MaterialIndexes)
	{
		if (MatIdx >= SkMC->GetNumMaterials())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MatIdx);
			continue;
		}
		HighlightSkMC->SetMaterial(MatIdx, DynamicMaterial);
	}

	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
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
	if (HighlightSMC)
	{
		if (HighlightSMC->IsValidLowLevel())
		{
			//HighlightSMC->DestroyComponent();
			HighlightSMC->ConditionalBeginDestroy();
		}
	}

	if (HighlightSkMC)
	{
		if (HighlightSkMC->IsValidLowLevel())
		{
			//HighlightSkMC->DestroyComponent();
			HighlightSkMC->ConditionalBeginDestroy();
		}
	}

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

// Set dynamic material
void USLVizHighlightMarker::SetDynamicMaterial(const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable())
	{
		// Dynamic material is valid, check if the type differs of the required one
		if (Type != MaterialType)
		{
			// Destroy previous material and create a new one
			DynamicMaterial->ConditionalBeginDestroy();
			if (Type == ESLVizHighlightMarkerType::Additive)
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);				
			}
			else if (Type == ESLVizHighlightMarkerType::Translucent)
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
		// Create first material
		if (Type == ESLVizHighlightMarkerType::Additive)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);			
		}
		else if (Type == ESLVizHighlightMarkerType::Translucent)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightTranslucent, NULL);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Unknown material type, using default as translucent.. "), *FString(__FUNCTION__), __LINE__);
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);
		}
	}

	DynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
	MaterialType = Type;
}
