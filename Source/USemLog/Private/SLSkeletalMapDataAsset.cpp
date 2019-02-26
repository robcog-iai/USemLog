// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalMapDataAsset.h"

// Ctor
USLSkeletalMapDataAsset::USLSkeletalMapDataAsset()
{
	bRefresh = false;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalMapDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalMapDataAsset, SkeletalMesh))
	{
		USLSkeletalMapDataAsset::UpdateData();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalMapDataAsset, bRefresh))
	{
		USLSkeletalMapDataAsset::UpdateData();
		bRefresh = false;
	}
}
#endif // WITH_EDITOR

// Update the data
void USLSkeletalMapDataAsset::UpdateData()
{
	if (SkeletalMesh)
	{
		// Empty previous map
		TMap<FName, FString> TempBoneClass = BoneClasses;
		BoneClasses.Empty();

		// Create a temporary skeletal mesh component from the mesh to read the bone names
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
