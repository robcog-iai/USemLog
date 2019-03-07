// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionViewComponent.h"

// Sets default values for this component's properties
USLVisionViewComponent::USLVisionViewComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLVisionViewComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	USLVisionViewComponent::SetSemanticOwnerAndData();

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	//if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVisionCameraComponent, SkeletalDataAsset))
	//{
	//}
}
#endif // WITH_EDITOR

// Clear all data
void USLVisionViewComponent::ClearData()
{
	SemanticOwner = nullptr;
	OwnerSemanticData.Clear();
	bInit = false;
}

// Init the component for runtime
bool USLVisionViewComponent::Init()
{
	return bInit = bInit ? true : USLVisionViewComponent::SetSemanticOwnerAndData();
}

// Set the semantic parent, returns true if already set
bool USLVisionViewComponent::SetSemanticOwnerAndData()
{
	if (OwnerSemanticData.IsSet())
	{
		return true; // Semantic data already set
	}
	else
	{
		// Make sure any prev data is deleted
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
					TEXT(__FUNCTION__), __LINE__);
				return false;
			}
		}
	}
}