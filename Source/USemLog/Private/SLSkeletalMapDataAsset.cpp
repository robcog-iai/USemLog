// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalMapDataAsset.h"

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
		// Empty previous map
		BoneClassMap.Empty();

		// Create a temporary skeletal mesh component from the mesh to read the bone names
		USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(this);
		SkelMeshComp->SetSkeletalMesh(SkeletalMesh);
		TArray<FName> BoneNames;
		SkelMeshComp->GetBoneNames(BoneNames);
		for (auto Name : BoneNames)
		{
			BoneClassMap.Add(Name, "");
		}
		SkelMeshComp->DestroyComponent();
	}
}
#endif // WITH_EDITOR


