// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBoneIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Skeletal/SLSkeletalDataComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Ctor
USLBoneIndividual::USLBoneIndividual()
{
	MaterialIndex = INDEX_NONE;
	BoneIndex = INDEX_NONE;
	//BoneName = NAME_None;
}

// Called before destroying the object.
void USLBoneIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLBoneIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	//Init();
}

// Set the parameters required when initalizing the individual
bool USLBoneIndividual::PreInit(int32 NewBoneIndex, bool bReset)
{
	if (bReset)
	{
		bIsPreInit = false;
	}

	if (IsPreInit())
	{
		return true;
	}

	BoneIndex = NewBoneIndex;
	ImportTagType += "Bone" + FString::FromInt(BoneIndex);
	bIsPreInit = true;
	return true;
}

// Set pointer to the semantic owner
bool USLBoneIndividual::Init(bool bReset)
{
	if (!IsPreInit())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot init individual %s, pre init need to be called right after creation.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

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
bool USLBoneIndividual::Load(bool bReset, bool bTryImport)
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

//// Export bone values
//bool USLBoneIndividual::ExportValues(bool bOverwrite)
//{
//	if (!IsInit())
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init, cannot export values.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName());
//		return false;
//	}
//
//	bool bNewValue = false;
//	FName BoneName = SkeletalMeshComponent->GetBoneName(BoneIndex);
//	if (FSLBoneData* BoneData = SkelDataComp->SemanticBonesData.Find(BoneName))
//	{
//		/* Id */
//		if (IsIdValueSet())
//		{
//			if (BoneData->Id.IsEmpty() || bOverwrite)
//			{
//				BoneData->Id = Id;
//				bNewValue = true;
//			}
//		}
//
//		/* Class */
//		if (IsClassValueSet())
//		{
//			if (BoneData->Class.IsEmpty() || bOverwrite)
//			{
//				BoneData->Class = Class;
//				bNewValue = true;
//			}
//		}
//
//		/* OId */
//		if (IsOIdValueSet())
//		{
//			if (BoneData->OId.IsEmpty() || bOverwrite)
//			{
//				BoneData->OId = OId;
//				bNewValue = true;
//			}
//		}
//
//		/* VisualMask */
//		if (IsVisualMaskValueSet())
//		{
//			if (BoneData->VisualMask.IsEmpty() || bOverwrite)
//			{
//				BoneData->VisualMask = VisualMask;
//				bNewValue = true;
//			}
//		}
//
//		/* CalibratedVisualMask */
//		if (IsCalibratedVisualMasValueSet())
//		{
//			if (BoneData->CalibratedVisualMask.IsEmpty() || bOverwrite)
//			{
//				BoneData->CalibratedVisualMask = CalibratedVisualMask;
//				bNewValue = true;
//			}
//		}
//
//		/* MaterialIndex */
//		if (HasValidMaterialIndex())
//		{
//			if (BoneData->MaterialIndex == INDEX_NONE || bOverwrite)
//			{
//				BoneData->MaterialIndex = MaterialIndex;
//				bNewValue = true;
//			}
//		}
//
//		/* BoneIndex */
//		if (HasValidBoneIndex())
//		{
//			if (BoneData->BoneIndex == INDEX_NONE || bOverwrite)
//			{
//				BoneData->BoneIndex = BoneIndex;
//				bNewValue = true;
//			}
//		}
//	}
//	return bNewValue;
//}
//
//// Import bone values
//bool USLBoneIndividual::ImportValues(bool bOverwrite)
//{
//	// TODO use tags (e.g. SemLogBone+BoneIndex:Id;Class;Oid;)
//	if (!IsInit())
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init, cannot import values.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName());
//		return false;
//	}
//
//	bool bNewValue = false;
//	FName BoneName = SkeletalMeshComponent->GetBoneName(BoneIndex);
//	if (FSLBoneData* BoneData = SkelDataComp->SemanticBonesData.Find(BoneName))
//	{
//		/* Id */
//		if (!IsIdValueSet() || bOverwrite)
//		{
//			const FString PrevVal = Id;
//			SetIdValue(BoneData->Id);
//			if (!Id.Equals(PrevVal))
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* Class */
//		if (!IsClassValueSet() || bOverwrite)
//		{
//			const FString PrevVal = Class;
//			SetClassValue(BoneData->Class);
//			if (!Class.Equals(PrevVal))
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* OId */
//		if (!IsOIdValueSet() || bOverwrite)
//		{
//			const FString PrevVal = OId;
//			SetOIdValue(BoneData->OId);
//			if (!OId.Equals(PrevVal))
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* VisualMask */
//		if (!IsVisualMaskValueSet() || bOverwrite)
//		{
//			const FString PrevVal = VisualMask;
//			SetVisualMaskValue(BoneData->VisualMask);
//			if (!VisualMask.Equals(PrevVal))
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* CalibratedVisualMask */
//		if (!IsCalibratedVisualMasValueSet() || bOverwrite)
//		{
//			const FString PrevVal = CalibratedVisualMask;
//			SetCalibratedVisualMaskValue(BoneData->CalibratedVisualMask);
//			if (!CalibratedVisualMask.Equals(PrevVal))
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* MaterialIndex */
//		if (!HasValidMaterialIndex() || bOverwrite)
//		{
//			const int32 PrevVal = MaterialIndex;
//			MaterialIndex = BoneData->MaterialIndex;
//			if (MaterialIndex != PrevVal)
//			{
//				bNewValue = true;
//			}
//		}
//
//		/* BoneIndex */
//		if (!HasValidBoneIndex() || bOverwrite)
//		{
//			const int32 PrevVal = BoneIndex;
//			BoneIndex = BoneData->BoneIndex;
//			if (BoneIndex != PrevVal)
//			{
//				bNewValue = true;
//			}
//		}
//	}
//	return bNewValue;
//}

// Apply visual mask material
bool USLBoneIndividual::ApplyMaskMaterials(bool bPrioritizeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		SkeletalMeshComponent->SetMaterial(MaterialIndex, VisualMaskDynamicMaterial);
		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLBoneIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		// Applied in parent
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Calculate current bone transform
bool USLBoneIndividual::CacheCurrentBoneTransform()
{
	if (IsInit())
	{
		CachedTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
		return true;
	}
	return false;
}

// Private init implementation
bool USLBoneIndividual::InitImpl()
{
	// Make sure the visual mesh is set
	if (HasValidSkeletalMesh() || SetSkeletalMesh())
	{
		// TODO set parent and child bone individual
		return true;
	}
	return false;

	if (!HasValidSkeletalMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no skeletal mesh.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	if (!HasValidSkeletalDataComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no skeletal data.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	if (!HasValidMaterialIndex())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no material index.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	if (!HasValidBoneIndex())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no bone index.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	return true;
}

// Private load implementation
bool USLBoneIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Get class name, virtual since each invidiual type will have different name
FString USLBoneIndividual::CalcDefaultClassValue() const
{
	if (IsInit())
	{
		if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
		{
			if (SkI->HasValidSkeletalDataAsset())
			{
				if (FString* BoneClassValue = SkI->SkeletalDataAsset->BoneIndexClass.Find(BoneIndex))
				{
					if (!BoneClassValue->IsEmpty())
					{
						return *BoneClassValue;
					}
				}
			}
		}
	}
	return GetTypeName();
}

// Clear all values of the individual
void USLBoneIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLBoneIndividual::LoadReset()
{
	Super::LoadReset();
}

// Clear any bound delegates (called when init is reset)
void USLBoneIndividual::ClearDelegateBounds()
{
}

// Set the skeletal actor as parent
bool USLBoneIndividual::SetParentActor()
{
	if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
	{
		if (UActorComponent* AC = Cast<UActorComponent>(SkI->GetOuter()))
		{
			if (AActor* CompOwner = Cast<AActor>(AC->GetOuter()))
			{
				ParentActor = CompOwner;
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's third outer should be the parent actor.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's second outer should be an actor component.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer should be a skeletal individual.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if skeleltal bone description component is available
bool USLBoneIndividual::HasValidSkeletalDataComponent() const
{
	return SkelDataComp && SkelDataComp->IsValidLowLevel() && !SkelDataComp->IsPendingKill();
}

// Check if the material index is valid
bool USLBoneIndividual::HasValidMaterialIndex() const
{
	return HasValidSkeletalMesh()
		&& MaterialIndex != INDEX_NONE
		&& MaterialIndex < SkeletalMeshComponent->GetNumMaterials();
}

// Check if the bone index is valid
bool USLBoneIndividual::HasValidBoneIndex() const
{
	return HasValidSkeletalMesh()
		&& BoneIndex != INDEX_NONE
		&& BoneIndex < SkeletalMeshComponent->GetNumBones();
}

// Check if the static mesh component is set
bool USLBoneIndividual::HasValidSkeletalMesh() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set sekeletal mesh
bool USLBoneIndividual::SetSkeletalMesh()
{
	// Outer should be the skeletal individual
	if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
	{
		if (SkI->HasValidSkeletalMesh())
		{
			SkeletalMeshComponent = SkI->SkeletalMeshComponent;
			return true;
		}
	}
	return false;
}
