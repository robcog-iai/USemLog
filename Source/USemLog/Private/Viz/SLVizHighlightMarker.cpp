// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizHighlightMarker.h"
#include "Viz/SLVizMarkerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizHighlightMarker::USLVizHighlightMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	LoadAssets();
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
		HighlightSkMC->DestroyComponent();
	}

	if (HighlightSMC->SetStaticMesh(SMC->GetStaticMesh()))
	{
		SetWorldTransform(SMC->GetComponentTransform());
		HighlightSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		UMaterialInstanceDynamic* DynMat = nullptr;
		if (Type == ESLVizHighlightMarkerType::Additive)
		{
			DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightAdditive, NULL);
		}
		else if (Type == ESLVizHighlightMarkerType::Translucent)
		{
			DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightTranslucent, NULL);
		}

		if(DynMat)
		{
			DynMat->SetVectorParameterValue(FName("Color"), Color);

			for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
			{
				HighlightSMC->SetMaterial(MatIdx, DynMat);
			}
		}
	}
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
		HighlightSMC->DestroyComponent();
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

	UMaterialInstanceDynamic* DynMat = nullptr;
	if (Type == ESLVizHighlightMarkerType::Additive)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightAdditive, NULL);
	}
	else if (Type == ESLVizHighlightMarkerType::Translucent)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightTranslucent, NULL);
	}

	if (DynMat)
	{
		DynMat->SetVectorParameterValue(FName("Color"), Color);

		for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
		{
			HighlightSkMC->SetMaterial(MatIdx, DynMat);
		}
	}	
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
		HighlightSMC->DestroyComponent();
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

	UMaterialInstanceDynamic* DynMat = nullptr;
	if (Type == ESLVizHighlightMarkerType::Additive)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightAdditive, NULL);
	}
	else if (Type == ESLVizHighlightMarkerType::Translucent)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightTranslucent, NULL);
	}

	if (DynMat)
	{
		DynMat->SetVectorParameterValue(FName("Color"), Color);
		HighlightSkMC->SetMaterial(MaterialIndex, DynMat);
	}

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
		HighlightSMC->DestroyComponent();
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

	UMaterialInstanceDynamic* DynMat = nullptr;
	if (Type == ESLVizHighlightMarkerType::Additive)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightAdditive, NULL);
	}
	else if (Type == ESLVizHighlightMarkerType::Translucent)
	{
		DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightTranslucent, NULL);
	}

	if (DynMat)
	{
		DynMat->SetVectorParameterValue(FName("Color"), Color);
		for (int32 MatIdx : MaterialIndexes)
		{
			if (MatIdx >= SkMC->GetNumMaterials())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, MatIdx);
				continue;
			}
			HighlightSkMC->SetMaterial(MatIdx, DynMat);
		}		
	}
}

// Clear marker by notifing parent manager
bool USLVizHighlightMarker::DestroyThroughManager()
{
	if (ASLVizMarkerManager* Manager = Cast<ASLVizMarkerManager>(GetOwner()))
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
		HighlightSMC->DestroyComponent();
	}

	if (HighlightSkMC)
	{
		HighlightSkMC->DestroyComponent();
	}

	Super::DestroyComponent(bPromoteChildren);
}

// Load highlight material assets
void USLVizHighlightMarker::LoadAssets()
{
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialHighlightAdditiveAsset(TEXT("Material'/UViz/M_HighlightDynamicColorAdditive.M_HighlightDynamicColorAdditive'"));
	MaterialHighlightAdditive = MaterialHighlightAdditiveAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialHighlightTranslucentAsset(TEXT("Material'/UViz/M_HighlightDynamicColorTranslucent.M_HighlightDynamicColorTranslucent'"));
	MaterialHighlightTranslucent = MaterialHighlightTranslucentAsset.Object;
}