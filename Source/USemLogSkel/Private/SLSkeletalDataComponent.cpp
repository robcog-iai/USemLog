// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalDataComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tags.h"

// Sets default values for this component's properties
USLSkeletalDataComponent::USLSkeletalDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bInit = false;
	bReloadFromDataAssetButton = false;
	bRefresh = false;
	bClearAllDataButton = false;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalDataComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, SkeletalDataAsset))
	{
		LoadFromDataAsset();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bReloadFromDataAssetButton))
	{
		LoadFromDataAsset();
		bReloadFromDataAssetButton = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bRefresh))
	{
		Refresh();
		bRefresh = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bClearAllDataButton))
	{
		ClearData();
		bClearAllDataButton = false;
	}
}
#endif // WITH_EDITOR

// Init the component for runtime
bool USLSkeletalDataComponent::Init()
{
	if (!bInit)
	{
		if (SetSemanticOwnerData() && SetSkeletalParent())
		{
			bInit = true;
		}
		else 
		{
			bInit = false;
		}
	}
	return bInit;
}

// Clear and re-load data from data asset
void USLSkeletalDataComponent::LoadFromDataAsset()
{
	// Remove any previous data
	ClearData();

	// Continue if attachment parent is a skeletal mesh component
	if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
	{
		// Set the pointer to the skeletal mesh parent
		SkeletalMeshParent = SkMC;

		// Update from data asset
		if (SkeletalDataAsset)
		{
			// Iterate bone classes from the data asset, ignore empty classes
			for (const auto& BoneClassPair : SkeletalDataAsset->BoneClasses)
			{
				const FString BoneClass = BoneClassPair.Value;
				const FName BoneName = BoneClassPair.Key;
				if (!BoneClass.IsEmpty())
				{
					// Create new data
					FSLBoneData BoneData;
					BoneData.Class = BoneClass;
					BoneData.MaskMaterialIndex = SkMC->GetMaterialIndex(FName(*BoneClass));

					if (BoneData.MaskMaterialIndex == INDEX_NONE)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Class %s is not part of a material slot.."),
							*FString(__func__), __LINE__, *BoneClass);
					}

					//// Find the material slot with the bone class name
					//if (FSkeletalMaterial* SkelMat = SkMC->SkeletalMesh->Materials.FindByPredicate(
					//	[&BoneClass](const FSkeletalMaterial& InMat)
					//	{return InMat.MaterialSlotName.ToString().Equals(BoneClass); }))
					//{
					//	BoneData.MaskMaterial = SkelMat->MaterialInterface;
					//}

					SemanticBonesData.Add(BoneName, BoneData);
				}
			}

			// Set owner and the semantic bone data
			SetSemanticOwnerData();

			// Make sure that the non-annotated bones have an empty semantics structure
			SetDataForAllBones();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Parent is not a USkeletalMeshComponent.."), *FString(__func__), __LINE__);
	}
}

// Refresh data, update material indexes remove invalid data
void USLSkeletalDataComponent::Refresh()
{
	// Continue if attachment parent is a skeletal mesh component
	if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
	{
		// Set the pointer to the skeletal mesh parent
		SkeletalMeshParent = SkMC;

		// Iterate bone classes from the data asset, ignore empty classes
		for (auto& BoneDataPair : SemanticBonesData)
		{
			// Update the bone index
			BoneDataPair.Value.MaskMaterialIndex = SkMC->GetMaterialIndex(FName(*BoneDataPair.Value.Class));

			if (BoneDataPair.Value.MaskMaterialIndex == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Class %s is not part of a material slot.."),
					*FString(__func__), __LINE__, *BoneDataPair.Value.Class);
			}
		}

		// Set owner and the semantic bone data
		SetSemanticOwnerData();

		// Make sure that the non-annotated bones have an empty semantics structure
		SetDataForAllBones();
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Parent is not a USkeletalMeshComponent.."), *FString(__func__), __LINE__);
	}
}


// Clear all data
void USLSkeletalDataComponent::ClearData()
{
	SemanticBonesData.Empty();
	AllBonesData.Empty();
	OwnerSemanticData.Clear();
	SkeletalMeshParent = nullptr;
	SemanticOwner = nullptr;
	bInit = false;
}

// Set the skeletal parent, returns true if already set
bool USLSkeletalDataComponent::SetSkeletalParent()
{
	if (SkeletalMeshParent)
	{
		// Mesh parent already set
		return true; 
	}
	else
	{
		if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
		{
			SkeletalMeshParent = SkMC;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d GetAttachParent() is not of type USkeletalMeshComponent.."),
				*FString(__func__), __LINE__);
			return false;
		}
	}
}

// Set the semantic parent data, returns true if already set
bool USLSkeletalDataComponent::SetSemanticOwnerData()
{
	if (OwnerSemanticData.IsSet())
	{
		return true; // Semantic data already set
	}
	else
	{
		// Make sure any pre data is deleted
		OwnerSemanticData.Clear();

		// Check if the attachment is the semantic owner
		FString Id = FTags::GetValue(GetAttachParent(), "SemLog", "Id");
		FString Class = FTags::GetValue(GetAttachParent(), "SemLog", "Class");
		
		if (!Id.IsEmpty() && !Class.IsEmpty())
		{
			SemanticOwner = GetAttachParent();
			OwnerSemanticData.Set(SemanticOwner, Id, Class);
			return true;
		}
		else
		{
			// Check if the owner is the semantic parent
			Id = FTags::GetValue(GetOwner(), "SemLog", "Id");
			Class = FTags::GetValue(GetOwner(), "SemLog", "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				SemanticOwner = GetOwner();
				OwnerSemanticData.Set(SemanticOwner, Id, Class);
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d None of GetAttachParent() or GetOwner() is a semantic owner.."),
					*FString(__func__), __LINE__);
				return false;
			}
		}
	}
}

// Set data for all the bones (empty for the ones without semantics)
void USLSkeletalDataComponent::SetDataForAllBones()
{
	if (SkeletalMeshParent)
	{
		// Merge the semantic bones data to the into the rest of the skeletal bones (these will have FSLBoneData empty/invalid)
		TArray<FName> AllBoneNames;
		SkeletalMeshParent->GetBoneNames(AllBoneNames);
		for (const auto& BoneName : AllBoneNames)
		{
			// Add bone with empty data
			if (SemanticBonesData.Contains(BoneName))
			{
				AllBonesData.Add(BoneName, SemanticBonesData[BoneName]);
			}
			else
			{
				AllBonesData.Add(BoneName, FSLBoneData());
			}
		}
	}
}
