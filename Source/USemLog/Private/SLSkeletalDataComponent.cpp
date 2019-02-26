// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSkeletalDataComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
USLSkeletalDataComponent::USLSkeletalDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bReloadData = false;
	bClearAllData = false;
}


// Called when the game starts
void USLSkeletalDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLSkeletalDataComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, LoadFromDataAsset))
	{
		USLSkeletalDataComponent::LoadData();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bReloadData))
	{
		USLSkeletalDataComponent::LoadData();
		bReloadData = false;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLSkeletalDataComponent, bClearAllData))
	{
		BonesData.Empty();
		bClearAllData = false;
	}
}
#endif // WITH_EDITOR

// Update the data
void USLSkeletalDataComponent::LoadData()
{
	// Continue if attachment parent is a skeletal mesh component
	if (USkeletalMeshComponent* SkMC = Cast<USkeletalMeshComponent>(GetAttachParent()))
	{
		// Update from data asset
		if (LoadFromDataAsset)
		{
			// Iterate bone classes, ignore empty classes
			for (const auto& BoneClassPair : LoadFromDataAsset->BoneClasses)
			{
				const FString BoneClass = BoneClassPair.Value;
				const FName BoneName = BoneClassPair.Key;
				if (!BoneClass.IsEmpty())
				{
					// Override any previous data for now
					if (BonesData.Contains(BoneName))
					{
						// Find the material slot with the bone class name
						if (FSkeletalMaterial* SkelMat = SkMC->SkeletalMesh->Materials.FindByPredicate([&BoneClass](const FSkeletalMaterial& InMat)
						{return InMat.MaterialSlotName.ToString().Equals(BoneClass); }))
						{
							BonesData[BoneName].MaskMaterialInstance = Cast<UMaterialInstance>(SkelMat->MaterialInterface);
						}
					}
					else
					{
						FSLBoneData BoneData;
						BoneData.Class = BoneClass;
						// Find the material slot with the bone class name
						if (FSkeletalMaterial* SkelMat = SkMC->SkeletalMesh->Materials.FindByPredicate([&BoneClass](const FSkeletalMaterial& InMat)
						{return InMat.MaterialSlotName.ToString().Equals(BoneClass); }))
						{
							BoneData.MaskMaterialInstance = Cast<UMaterialInstance>(SkelMat->MaterialInterface);
						}
						BonesData.Add(BoneName, BoneData);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Parent is not a USkeletalMeshComponent.."), TEXT(__FUNCTION__), __LINE__);
	}
}

//// Check if the bone was previously set
//if (TempBoneData.Contains(Name))
//{
//	// Check if it has a mask material reference
//	if (TempBoneData[Name].MaskMaterialInstance)
//	{
//		BoneData.Add(Name, TempBoneData[Name]);
//	}
//	else
//	{
//		// Search for the material
//		if (FSkeletalMaterial* FoundMat = SkeletalMesh->Materials.FindByPredicate([&Name](const FSkeletalMaterial& InMat)
//		{return Name == InMat.MaterialSlotName;	}))
//		{
//			TempBoneData[Name].MaskMaterialInstance = Cast<UMaterialInstance>(FoundMat->MaterialInterface);
//		}
//	}
//}
//else
//{
//	BoneData.Add(Name, FSLBoneData());
//}


		//	// Copy the mapping from the data asset
		//	BoneClasses = LoadFromSkeletalMapDataAsset->BoneClasses;

		//	// Get bone names from parent skeletal mesh
		//	TArray<FName> ParentBoneNames;
		//	SkMC->GetBoneNames(ParentBoneNames);

		//	// Remove entries with bones not available in the parent or with empty classes
		//	for (auto MapItr(BoneClasses.CreateIterator()); MapItr; ++MapItr)
		//	{
		//		if (!ParentBoneNames.Contains(MapItr.Key()) || MapItr.Value().IsEmpty())
		//		{
		//			MapItr.RemoveCurrent();
		//		}
		//	}


		//	//// Empty previous map
		//	//TMap<FName, FString> TempBoneClass = BoneClassMap;
		//	//BoneClassMap.Empty();

		//	//// Create a temporary skeletal mesh component from the mesh to read the bone names
		//	//USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(this);
		//	//SkelMeshComp->SetSkeletalMesh(SkeletalMesh);
		//	//TArray<FName> BoneNames;
		//	//SkelMeshComp->GetBoneNames(BoneNames);
		//	//for (const auto& Name : BoneNames)
		//	//{
		//	//	if (TempBoneClass.Contains(Name))
		//	//	{
		//	//		BoneClassMap.Add(Name, TempBoneClass[Name]);
		//	//	}
		//	//	else
		//	//	{
		//	//		BoneClassMap.Add(Name, "");
		//	//	}
		//	//}
		//	//SkelMeshComp->DestroyComponent();
		//}

		//if (SkMC->SkeletalMesh)
		//{
		//	// Load the material instances used for masking 
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \n\t SkMC=%s; \n\t path=%s \n****\n \n\t SkMC->SkeletalMesh=%s; \n\t path=%s"),
		//		TEXT(__FUNCTION__), __LINE__,
		//		*SkMC->GetFullName(), *SkMC->GetPathName(),
		//		*SkMC->SkeletalMesh->GetFullName(), *SkMC->SkeletalMesh->GetPathName() );

		//	for (const auto& M : SkMC->SkeletalMesh->Materials)
		//	{
		//		UE_LOG(LogTemp, Warning, TEXT("%s::%d MSlot=%s ISlot=%s;"), TEXT(__FUNCTION__), __LINE__,
		//			*M.MaterialSlotName.ToString(), *M.ImportedMaterialSlotName.ToString());
		//	}
		//	//FSkeletalMaterial M;
		//	
		//}