// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalMappingComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
USLSkeletalMappingComponent::USLSkeletalMappingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USLSkeletalMappingComponent::BeginPlay()
{
	Super::BeginPlay();
		
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalMappingComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalMappingComponent, LoadFromSkeletalMapDataAsset))
	{
		// 
		if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
		{
			if (LoadFromSkeletalMapDataAsset)
			{
				// Copy the mapping from the data asset
				BoneClassMap = LoadFromSkeletalMapDataAsset->BoneClassMap;

				// Get bone names from parent skeletal mesh
				TArray<FName> ParentBoneNames;
				SkMC->GetBoneNames(ParentBoneNames);
			
				// Remove entries with bones not available in the parent or with emtpy classes
				for (auto MapItr(BoneClassMap.CreateIterator()); MapItr; ++MapItr)
				{
					if (!ParentBoneNames.Contains(MapItr.Key()) || MapItr.Value().IsEmpty())
					{
						MapItr.RemoveCurrent();
					}
				}
			
				// Load the material instances used for masking 




				//// Empty previous map
				//TMap<FName, FString> TempBoneClass = BoneClassMap;
				//BoneClassMap.Empty();

				//// Create a temporary skeletal mesh component from the mesh to read the bone names
				//USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(this);
				//SkelMeshComp->SetSkeletalMesh(SkeletalMesh);
				//TArray<FName> BoneNames;
				//SkelMeshComp->GetBoneNames(BoneNames);
				//for (const auto& Name : BoneNames)
				//{
				//	if (TempBoneClass.Contains(Name))
				//	{
				//		BoneClassMap.Add(Name, TempBoneClass[Name]);
				//	}
				//	else
				//	{
				//		BoneClassMap.Add(Name, "");
				//	}
				//}
				//SkelMeshComp->DestroyComponent();
			}
		}
		else 
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Parent is not a USkeletalMeshComponent.."), TEXT(__FUNCTION__), __LINE__);
		}
	}
}
#endif // WITH_EDITOR
