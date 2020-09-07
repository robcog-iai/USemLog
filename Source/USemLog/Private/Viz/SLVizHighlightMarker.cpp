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
	HighlightSkelMC = nullptr;
	DynamicMaterial = nullptr;
	MaterialType = ESLVizHighlightMarkerType::NONE;
}

// Highlight the given static mesh by creating a clone
void USLVizHighlightMarker::Init(UStaticMeshComponent* SMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	// (If previously set) destroy skeletal component
	ClearSkeletalMeshComponent();

	// Check if a new static mesh component needs to be created
	if (!HighlightSMC || !HighlightSMC->IsValidLowLevel() || HighlightSMC->IsPendingKillOrUnreachable())
	{
		HighlightSMC = NewObject<UStaticMeshComponent>(this);
		HighlightSMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSMC->RegisterComponent();
	}
	else
	{
		HighlightSMC->EmptyOverrideMaterials();
	}

	// Set the static mesh visual
	if (HighlightSMC->SetStaticMesh(SMC->GetStaticMesh()))
	{
		//SetWorldTransform(SMC->GetComponentTransform());
		HighlightSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightSMC->bSelectable = false;

		SetDynamicMaterial(Color, Type);
		for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSMC->SetMaterial(MatIdx, DynamicMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not set the static mesh.."), *FString(__FUNCTION__), __LINE__);
	}

	AttachToComponent(SMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	// (If previously set) destroy static mesh component
	ClearStaticMeshComponent();

	// Check if a new skeletal mesh component needs to be created
	if (!HighlightSkelMC || !HighlightSkelMC->IsValidLowLevel() || HighlightSkelMC->IsPendingKillOrUnreachable())
	{
		HighlightSkelMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkelMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSkelMC->RegisterComponent();
	}
	else
	{
		HighlightSkelMC->EmptyOverrideMaterials();
	}

	// Set the skeletal mesh visual
	HighlightSkelMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	HighlightSkelMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightSkelMC->bSelectable = false;
	//SetWorldTransform(SkMC->GetComponentTransform());

	//HighlightSkelMC->BoneSpaceTransforms = SkMC->BoneSpaceTransforms;
	//HighlightSkelMC->MarkRefreshTransformDirty();

	TArray<FName> BoneNames;
	SkMC->GetBoneNames(BoneNames);
	for (const auto Name : BoneNames)
	{
		HighlightSkelMC->SetBoneTransformByName(Name, SkMC->GetBoneTransform(SkMC->GetBoneIndex(Name)), EBoneSpaces::WorldSpace);
	}

	SetDynamicMaterial(Color, Type);
	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial);
	}


	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bone (material index) skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	// (If previously set) destroy static mesh component
	ClearStaticMeshComponent();

	if (MaterialIndex >= SkMC->GetNumMaterials())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MaterialIndex);
		return;
	}

	if (!HighlightSkelMC)
	{
		HighlightSkelMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkelMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		HighlightSkelMC->RegisterComponent();
	}
	else
	{
		HighlightSkelMC->EmptyOverrideMaterials();
	}

	HighlightSkelMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	HighlightSkelMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightSkelMC->bSelectable = false;
	//SetWorldTransform(SkMC->GetComponentTransform());

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

	SetDynamicMaterial(Color, Type);
	HighlightSkelMC->SetMaterial(MaterialIndex, DynamicMaterial);

	// TODO this will not work if the skeletal component is animated
	AttachToComponent(SkMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

// Highlight the given bones (material indexes) skeletal mesh by creating a clone
void USLVizHighlightMarker::Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FLinearColor& Color, ESLVizHighlightMarkerType Type)
{
	if (!HighlightSkelMC)
	{
		HighlightSkelMC = NewObject<UPoseableMeshComponent>(this);
		HighlightSkelMC->RegisterComponent();
		HighlightSkelMC->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		HighlightSkelMC->EmptyOverrideMaterials();
	}

	if (HighlightSMC)
	{
		if (HighlightSMC->IsValidLowLevel())
		{
			//HighlightSMC->DestroyComponent();
			HighlightSMC->ConditionalBeginDestroy();
		}
	}

	HighlightSkelMC->SetSkeletalMesh(SkMC->SkeletalMesh);
	SetWorldTransform(SkMC->GetComponentTransform());

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

	SetDynamicMaterial(Color, Type);
	for (int32 MatIdx : MaterialIndexes)
	{
		if (MatIdx >= SkMC->GetNumMaterials())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MatIdx);
			continue;
		}
		HighlightSkelMC->SetMaterial(MatIdx, DynamicMaterial);
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

// Set dynamic material
void USLVizHighlightMarker::SetDynamicMaterial(const FLinearColor& Color, ESLVizHighlightMarkerType NewType)
{
	const bool bIsMaterialValid = DynamicMaterial && DynamicMaterial->IsValidLowLevel() && !DynamicMaterial->IsPendingKillOrUnreachable();
	if (bIsMaterialValid)
	{
		const bool bIsValidMaterialOfTheSameType = NewType != MaterialType;
		if (!bIsValidMaterialOfTheSameType)
		{
			// Destroy previous material and create a new one
			DynamicMaterial->ConditionalBeginDestroy();
			if (NewType == ESLVizHighlightMarkerType::Additive)
			{
				DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);				
			}
			else if (NewType == ESLVizHighlightMarkerType::Translucent)
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
		if (NewType == ESLVizHighlightMarkerType::Additive)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(VizAssetsContainer->MaterialHighlightAdditive, NULL);			
		}
		else if (NewType == ESLVizHighlightMarkerType::Translucent)
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
	MaterialType = NewType;
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

// Clear the skeletal mesh component
void USLVizHighlightMarker::ClearSkeletalMeshComponent()
{
	if (HighlightSkelMC && HighlightSkelMC->IsValidLowLevel() && !HighlightSkelMC->IsPendingKillOrUnreachable())
	{
		//HighlightSkelMC->ConditionalBeginDestroy();
		HighlightSkelMC->DestroyComponent();
	}
	HighlightSkelMC = nullptr;
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
