// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalDataAsset.h"

// Ctor
USLSkeletalDataAsset::USLSkeletalDataAsset()
{
	bReloadData = false;
	bClearEmptyEntries = false;
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataAsset, bClearEmptyEntries))
	{
		for (auto MapItr(BoneClasses.CreateIterator()); MapItr; ++MapItr)
		{
			if (MapItr->Value.IsEmpty())
			{
				MapItr.RemoveCurrent();
			}
		}
		bClearEmptyEntries = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataAsset, bClearAllData))
	{
		BoneClasses.Empty();
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

		// Create a temporary USkeletalMeshComponent to be able to read the bone names
		USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(this);
		SkelMeshComp->SetSkeletalMesh(SkeletalMesh);
		TArray<FName> BoneNames;
		SkelMeshComp->GetBoneNames(BoneNames);
		for (const auto& Name : BoneNames)
		{
			if (TempBoneClass.Contains(Name))
			{
				BoneClasses.Add(Name, TempBoneClass[Name]);
			}
			else
			{
				BoneClasses.Add(Name, "");
			}
		}
		SkelMeshComp->DestroyComponent();
	}
}
