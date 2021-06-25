// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Skeletal/SLSkeletalDataAsset.h"
#include "Components/SkeletalMeshComponent.h"

// Ctor
USLSkeletalDataAsset::USLSkeletalDataAsset()
{
	bReloadData = false;
	bClearAllData = false;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataAsset, SkeletalMesh))
	{
		USLSkeletalDataAsset::LoadData();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataAsset, bReloadData))
	{
		USLSkeletalDataAsset::LoadData();
		bReloadData = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataAsset, bClearAllData))
	{
		BoneClasses.Empty();
		BoneIndexClass.Empty();
		bClearAllData = false;
	}
}
#endif // WITH_EDITOR

// Update the data
void USLSkeletalDataAsset::LoadData()
{
	if (SkeletalMesh)
	{
		// Empty previous map
		TMap<FName, FString> TempBoneClass = BoneClasses;
		BoneClasses.Empty();
		BoneIndexClass.Empty();

		// Create a temporary USkeletalMeshComponent to be able to read the bone names
		USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(this);
		SkelMeshComp->SetSkeletalMesh(SkeletalMesh);
		TArray<FName> BoneNames;
		SkelMeshComp->GetBoneNames(BoneNames);
		for (const auto& Name : BoneNames)
		{
			int32 CurrBoneIndex = SkelMeshComp->GetBoneIndex(Name);
			if (CurrBoneIndex != INDEX_NONE)
			{
				if (TempBoneClass.Contains(Name))
				{
					BoneClasses.Add(Name, TempBoneClass[Name]);
					BoneIndexClass.Add(CurrBoneIndex, TempBoneClass[Name]);
				}
				else
				{
					BoneClasses.Add(Name, "");
					BoneIndexClass.Add(CurrBoneIndex, "");
				}				
			}
		}
		SkelMeshComp->DestroyComponent();		
	}
}
