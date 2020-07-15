// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLBoneIndividual.h"
#include "Individuals/SLVirtualBoneIndividual.h"
#include "Individuals/SLIndividualComponent.h"
#include "Skeletal/SLSkeletalDataAsset.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"

#include "Skeletal/SLSkeletalDataComponent.h"

// Ctor
USLSkeletalIndividual::USLSkeletalIndividual()
{
	SkeletalMeshComponent = nullptr;
	SkeletalDataAsset = nullptr;
}

// Called before destroying the object.
void USLSkeletalIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
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

// Save values externally
bool USLSkeletalIndividual::ExportValues(bool bOverwrite)
{
	bool RetVal = Super::ExportValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ExportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ExportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	return RetVal;
}


// Load values externally
bool USLSkeletalIndividual::ImportValues(bool bOverwrite)
{
	bool RetVal = Super::ImportValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ImportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ImportValues(bOverwrite))
		{
			RetVal = true;
		}
	}
	return RetVal;
}

// Clear exported values
bool USLSkeletalIndividual::ClearExportedValues()
{
	bool RetVal = Super::ClearExportedValues();
	for (const auto& BI : BoneIndividuals)
	{
		if (BI->ClearExportedValues())
		{
			RetVal = true;
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (VBI->ClearExportedValues())
		{
			RetVal = true;
		}
	}
	return false;
}

// Apply visual mask material
bool USLSkeletalIndividual::ApplyMaskMaterials(bool bIncludeChildren)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		if (bIncludeChildren)
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
			BI->ApplyOriginalMaterials();
		}

		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLSkeletalIndividual::CalcDefaultClassValue() const
{
	if (IsInit())
	{
		if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(ParentActor))
		{
			if (USkeletalMeshComponent* SkMC = SkMA->GetSkeletalMeshComponent())
			{
				FString ClassName = SkMC->SkeletalMesh->GetFullName();
				int32 FindCharPos;
				ClassName.FindLastChar('.', FindCharPos);
				ClassName.RemoveAt(0, FindCharPos + 1);
				ClassName.RemoveFromStart(TEXT("SK_"));
				return ClassName;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SkMC.."),
					*FString(__func__), __LINE__, *SkMA->GetName());
			}
		}
	}
	return GetTypeName();
}

// Clear all values of the individual
void USLSkeletalIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	DestroyBoneIndividuals();
	SkeletalMeshComponent = nullptr;
	SetIsInit(false);
	ClearDelegates();
}

// Clear all data of the individual
void USLSkeletalIndividual::LoadReset()
{
	Super::LoadReset();
	ResetBoneIndividuals();
}

// Private init implementation
bool USLSkeletalIndividual::InitImpl()
{
	// Make sure the visual mesh is set
	if (HasValidSkeletalMesh() || SetSkeletalMesh())
	{
		// Make sure there is a skeletal semantic data component
		if (HasValidSkeletalDataAsset() || SetSkeletalDataAsset())
		{
			// Make sure the bone individuals are created
			if (HasValidBones() || CreateBoneIndividuals())
			{
				return InitBoneIndividuals();
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
		return LoadBoneIndividuals();
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

// Check if skeleltal bone description asset is available
bool USLSkeletalIndividual::HasValidSkeletalDataAsset() const
{
	return SkeletalDataAsset && SkeletalDataAsset->IsValidLowLevel() && !SkeletalDataAsset->IsPendingKill();
}

// Set the skeletal data component from the individual component
bool USLSkeletalIndividual::SetSkeletalDataAsset()
{
	// Outer should be the individual component
	if (USLIndividualComponent* IC = Cast<USLIndividualComponent>(GetOuter()))
	{
		if (UDataAsset** DA = IC->OptionalDataAssets.Find(USLIndividualComponent::SkelDataAssetKey))
		{
			if (USLSkeletalDataAsset* SkDA = Cast<USLSkeletalDataAsset>(*DA))
			{
				SkeletalDataAsset = SkDA;
				return true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's has no skeletal data asset.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer is not an individual component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if all the bones are valid
bool USLSkeletalIndividual::HasValidBones() const
{
	// Bones are not valid without a skeletal mesh set
	if (!HasValidSkeletalMesh())
	{
		return false;
	}

	// The total number of bone individuals should coincide with the total skeletal bones
	if (BoneIndividuals.Num() + VirtualBoneIndividuals.Num() != SkeletalMeshComponent->GetNumBones())
	{
		return false;
	}

	// Make sure all individuals are valid
	bool bAllBonesAreValid = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsValidLowLevel() || BI->IsPendingKill() || !BI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s invalid bone found.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bAllBonesAreValid = false;
		}
	}
	for (const auto VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsValidLowLevel() || VBI->IsPendingKill() || !VBI->IsPreInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s invalid virtual bone found.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bAllBonesAreValid = false;
		}
	}
	return bAllBonesAreValid;
}

// Create new bone objects
bool USLSkeletalIndividual::CreateBoneIndividuals()
{
	// Destroy any previous individuals
	DestroyBoneIndividuals();

	if (!HasValidSkeletalMesh())
	{
		return false;
	}

	for (const auto& BoneData : SkeletalDataAsset->BoneIndexClass)
	{
		if (!BoneData.Value.IsEmpty())
		{
			USLBoneIndividual* BI = NewObject<USLBoneIndividual>(this);
			int32 MatIdx = SkeletalMeshComponent->GetMaterialIndex(FName(*BoneData.Value));
			if (MatIdx == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s bone will be invalid, no material slot found with the name: %s .."),
					*FString(__FUNCTION__), __LINE__, *GetFullName(), *BoneData.Value);
			}
			BI->PreInit(BoneData.Key, MatIdx);
			BoneIndividuals.Add(BI);
		}
		else
		{
			USLVirtualBoneIndividual* VBI = NewObject<USLVirtualBoneIndividual>(this);
			VBI->PreInit(BoneData.Key);
			VirtualBoneIndividuals.Add(VBI);
		}

		// TODO
		//SkeletalMeshComponent->Constraints
	}

	return HasValidBones();
}

// Call init on all bones, true if all succesfully init
bool USLSkeletalIndividual::InitBoneIndividuals()
{
	bool bAllBonesAreInit = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsInit() && !BI->Init())
		{
			bAllBonesAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d bone %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *BI->GetFullName());
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsInit() && !VBI->Init())
		{
			bAllBonesAreInit = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d virtual bone %s could not be init.."),
				*FString(__FUNCTION__), __LINE__, *VBI->GetFullName());
		}
	}
	return bAllBonesAreInit;
}

// Call load on all bones, true if all succesfully loaded
bool USLSkeletalIndividual::LoadBoneIndividuals()
{
	bool bAllBonesAreLoaded = true;
	for (const auto& BI : BoneIndividuals)
	{
		if (!BI->IsLoaded() && !BI->Load())
		{
			bAllBonesAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d bone %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *BI->GetFullName());
		}
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		if (!VBI->IsLoaded() && !VBI->Load())
		{
			bAllBonesAreLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d virtual bone %s could not be loaded.."),
				*FString(__FUNCTION__), __LINE__, *VBI->GetFullName());
		}
	}
	return bAllBonesAreLoaded;
}

// Destroy bone individuals
void USLSkeletalIndividual::DestroyBoneIndividuals()
{
	for (const auto& BI : BoneIndividuals)
	{
		BI->ConditionalBeginDestroy();
	}
	BoneIndividuals.Empty();
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		VBI->ConditionalBeginDestroy();
	}
	VirtualBoneIndividuals.Empty();
}

// Reset bone individuals
void USLSkeletalIndividual::ResetBoneIndividuals()
{
	for (const auto& BI : BoneIndividuals)
	{
		BI->Load(true);
	}
	for (const auto& VBI : VirtualBoneIndividuals)
	{
		VBI->Load(true);
	}
}
