// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLBoneIndividual.h"
#include "Skeletal/SLSkeletalDataComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"


// Ctor
USLSkeletalIndividual::USLSkeletalIndividual()
{
	SkeletalMeshComponent = nullptr;
	SkelDataComp = nullptr;
}

// Called before destroying the object.
void USLSkeletalIndividual::BeginDestroy()
{
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLSkeletalIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLSkeletalIndividual::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init() && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLSkeletalIndividual::Load(bool bReset, bool bTryImport)
{
	if (bReset)
	{
		LoadReset();
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load component individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}

// Apply visual mask material
bool USLSkeletalIndividual::ApplyMaskMaterials(bool bPrioritizeChildren)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		if (bPrioritizeChildren)
		{
			for (const auto& BI : BoneIndividuals)
			{
				BI->ApplyMaskMaterials();
			}
		}
		else
		{
			for (int32 MatIdx = 0; MatIdx < SkeletalMeshComponent->GetNumMaterials(); ++MatIdx)
			{
				SkeletalMeshComponent->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
			}
		}

		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLSkeletalIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		int32 MatIdx = 0;
		for (const auto& Mat : OriginalMaterials)
		{
			SkeletalMeshComponent->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}
		bIsMaskMaterialOn = false;

		// Bones share the same original materials with the skeletal parent
		for (const auto& BI : BoneIndividuals)
		{
			BI->bIsMaskMaterialOn = false;
		}

		return true;
	}
	return false;
}

// Clear all values of the individual
void USLSkeletalIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	DestroyBones();
	SkeletalMeshComponent = nullptr;
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLSkeletalIndividual::LoadReset()
{
	Super::LoadReset();
	ResetBones();
}

// Private init implementation
bool USLSkeletalIndividual::InitImpl()
{
	// Make sure the visual mesh is set
	if (HasValidSkeletalMesh() || SetSkeletalMesh())
	{
		// Make sure there is a skeletal semantic data component
		if (HasValidSkeletalDataComponent() || SetSkeletalDataComponent())
		{
			// Make sure the bone individuals are created
			if (HasValidBones() || CreateBones())
			{
				return InitAllBones();
			}
		}
	}
	return false;
}

// Private load implementation
bool USLSkeletalIndividual::LoadImpl(bool bTryImport)
{
	if (HasValidBones())
	{
		return LoadAllBones();
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s's bones should be valid here, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Check if the static mesh component is set
bool USLSkeletalIndividual::HasValidSkeletalMesh() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set sekeletal mesh
bool USLSkeletalIndividual::SetSkeletalMesh()
{
	if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(ParentActor))
	{
		if (USkeletalMeshComponent* SMC = SkMA->GetSkeletalMeshComponent())
		{
			SkeletalMeshComponent = SMC;
			OriginalMaterials = SMC->GetMaterials();
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SkeletalMeshComponent, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor is not a SkeletalMeshActor, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
}

// Check if skeleltal bone description component is available
bool USLSkeletalIndividual::HasValidSkeletalDataComponent() const
{
	return SkelDataComp && SkelDataComp->IsValidLowLevel() && !SkelDataComp->IsPendingKill();
}

// Set the skeletal data component
bool USLSkeletalIndividual::SetSkeletalDataComponent()
{
	if (UActorComponent* AC = ParentActor->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
	{
		SkelDataComp = CastChecked<USLSkeletalDataComponent>(AC);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor does not have a skeletal data component set.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Check if all the bones are valid
bool USLSkeletalIndividual::HasValidBones() const
{
	if (!BoneIndividuals.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s semantic bones array is empty.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;		
	}

	if (BoneIndividuals.Num() != SkelDataComp->SemanticBonesData.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s semantic bones array is out of sync with the skel data component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	for (const auto& Bone : BoneIndividuals)
	{
		if (!Bone->IsValidLowLevel() || Bone->IsPendingKill())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s invalid bone individual found, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	return true;
}

// Create new bone objects
bool USLSkeletalIndividual::CreateBones()
{
	for (const auto& BoneDataPair : SkelDataComp->SemanticBonesData)
	{
		USLBoneIndividual* BI = NewObject<USLBoneIndividual>(this);
		// TODO load from tags?
		BI->SetClassValue(BoneDataPair.Value.Class);
		BI->MaterialIndex = BoneDataPair.Value.MaterialIndex;
		BI->SkeletalMeshComponent = SkeletalMeshComponent;
		BI->SkelDataComp = SkelDataComp;
	}
	return true;
}

// Call init on all bones, true if all succesfully init
bool USLSkeletalIndividual::InitAllBones()
{
	bool bAllBonesAreInit = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsInit() && !BI->Init())
		{
			bAllBonesAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's bone %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *BI->GetName());
		}
	}
	return bAllBonesAreInit;
}

// Call load on all bones, true if all succesfully loaded
bool USLSkeletalIndividual::LoadAllBones()
{
	bool bAllBonesAreLoaded = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsLoaded() && !BI->Load())
		{
			bAllBonesAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's bone %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *BI->GetName());
		}
	}
	return bAllBonesAreLoaded;
}

// Destroy bone individuals
void USLSkeletalIndividual::DestroyBones()
{
	for (const auto BI : BoneIndividuals)
	{
		BI->InitReset();
		BI->ConditionalBeginDestroy();
	}
	BoneIndividuals.Empty();
}

// Reset bone individuals
void USLSkeletalIndividual::ResetBones()
{
	for (const auto BI : BoneIndividuals)
	{
		BI->LoadReset();
	}
}
