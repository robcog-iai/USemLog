// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalMapDataAsset.h"
#include "Animation/Rig.h"

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalMapDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalMapDataAsset, Skeleton))
	{
		UE_LOG(LogTemp, Warning, TEXT("\t %s::%d"), TEXT(__FUNCTION__), __LINE__);
		TArray<FName> NodesName;
		int32 NrOfBones  = Skeleton->GetMappedValidNodes(NodesName);
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d NR bones:%d"), TEXT(__FUNCTION__), __LINE__, NrOfBones);
		
		URig* Rig = Skeleton->GetRig();
		if (Rig)
		{
			TArray<FNode> Nodes = Rig->GetNodes();
			for (const auto& Node : Nodes)
			{
				FName NName = Node.Name;
				FName NPName = Node.ParentName;
				FString NDName = Node.DisplayName;
				UE_LOG(LogTemp, Warning, TEXT("\t\t %s::%d Name=%s, ParentName=%s, DisplayName=%s "),
					TEXT(__FUNCTION__), __LINE__, *NName.ToString(), *NPName.ToString(), *NDName);
				BoneSemanticNameMap.Add(Node.Name.ToString(), "NONE");			
			}
		}
		
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalMapDataAsset, SkeletalMesh))
	{
		
		BoneSemanticNameMap.Empty();
	}
}
#endif // WITH_EDITOR


