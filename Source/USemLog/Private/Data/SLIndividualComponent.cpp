// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividual.h"
#include "Data/SLSkeletalIndividual.h"
#include "Data/SLVisualIndividual.h"

// Utils
#include "Utils/SLUUid.h"

// Sets default values for this component's properties
USLIndividualComponent::USLIndividualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bOverwriteEditChanges = false;
	bSaveToTagButton = false;
	bLoadFromTagButton = false;

	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s"), *FString(__FUNCTION__), __LINE__, *GetName());
}

// Dtor
USLIndividualComponent::~USLIndividualComponent()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s"), *FString(__FUNCTION__), __LINE__, *GetName());
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	AActor* Owner = GetOwner();

	// Check if actor already has a semantic data component
	for (const auto AC : Owner->GetComponentsByClass(USLIndividualComponent::StaticClass()))
	{
		if (AC != this)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has a semantic data component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}

	// Set semantic individual type depending on owner
	if (UClass* IndividualClass = FSLIndividualUtils::CreateIndividualObject(this, Owner, SemanticIndividual))
	{
		// Cache the current individual class type
		ConvertTo = IndividualClass;
		if (SemanticIndividual)
		{
			SemanticIndividual->SetSemOwner(Owner);
		}
	}
	else
	{
		ConditionalBeginDestroy();
		return;
	}
	

	//if (Owner->IsA(AStaticMeshActor::StaticClass()))
	//{
	//	ConvertToSemanticIndividual = USLVisualIndividual::StaticClass();
	//	IndividualObj = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
	//	IndividualObj->SetSemOwner(Owner);
	//}
	//else if (Owner->IsA(ASkeletalMeshActor::StaticClass()))
	//{
	//	ConvertToSemanticIndividual = USLSkeletalIndividual::StaticClass();
	//	IndividualObj = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
	//	IndividualObj->SetSemOwner(Owner);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d unsuported actor type for generating a semantic individual component %s, self-destruction commenced.."),
	//		*FString(__FUNCTION__), __LINE__, *Owner->GetName());
	//	ConditionalBeginDestroy();
	//	return;
	//}
}

// Called when the game starts
void USLIndividualComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("%s::%d ******** %s .... "), *FString(__FUNCTION__), __LINE__, *GetName());
	// ...
	
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLIndividualComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Convert datatype
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, ConvertTo))
	{
		FSLIndividualUtils::ConvertIndividualObject(SemanticIndividual, ConvertTo);
	}

	/* Button workaround triggers */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bSaveToTagButton))
	{
		bSaveToTagButton = false;
		SaveToTag(bOverwriteEditChanges);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bLoadFromTagButton))
	{
		bLoadFromTagButton = false;
		LoadFromTag(bOverwriteEditChanges);
	}
}

// Save data to owners tag
void USLIndividualComponent::SaveToTag(bool bOverwrite)
{
	if (SemanticIndividual)
	{
		SemanticIndividual->SaveToTag(bOverwrite);
	}
}

// Load data from owners tag
void USLIndividualComponent::LoadFromTag(bool bOverwrite)
{
	if (SemanticIndividual)
	{
		SemanticIndividual->LoadFromTag(bOverwrite);
	}
}

// Write class name to the individual object
bool USLIndividualComponent::WriteClass(bool bOverwrite)
{
	if (USLIndividual* SLI = Cast<USLIndividual>(SemanticIndividual))
	{
		if (!SLI->HasClass() || bOverwrite)
		{
			SLI->SetClass(FSLIndividualUtils::GetIndividualClass(GetOwner()));
			return true;
		}
	}
	return false;
}

// Clear the class name
bool USLIndividualComponent::ClearClass()
{
	if (USLIndividual* SLI = Cast<USLIndividual>(SemanticIndividual))
	{
		if (SLI->HasClass())
		{
			SLI->SetClass("");
			return true;
		}
	}
	return false;
}


// Convert datat type object to the selected class type
void USLIndividualComponent::DoConvertDataType()
{
	if (ConvertTo)
	{		
		if (SemanticIndividual && !SemanticIndividual->IsPendingKill())
		{
			if(SemanticIndividual->StaticClass() != ConvertTo)
			{
				SemanticIndividual->ConditionalBeginDestroy();
				SemanticIndividual = NewObject<USLIndividualBase>(this, ConvertTo);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Converted datatype to %s (%s).."),
					*FString(__FUNCTION__), __LINE__, *ConvertTo->GetName(), *SemanticIndividual->StaticClass()->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Current datatype %s is of the same type as %s.."),
					*FString(__FUNCTION__), __LINE__, *SemanticIndividual->StaticClass()->GetName(), *ConvertTo->GetName());
			}
		}
		else
		{
			SemanticIndividual = NewObject<USLIndividualBase>(this, ConvertTo);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating new %s datatype (%s).."),
				*FString(__FUNCTION__), __LINE__, *ConvertTo->GetName(), *SemanticIndividual->StaticClass()->GetName());
		}
	}
	else if (SemanticIndividual && !SemanticIndividual->IsPendingKill())
	{
		SemanticIndividual->ConditionalBeginDestroy();
		SemanticIndividual = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Removed existing datatype.."),
			*FString(__FUNCTION__), __LINE__);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Nothing to remove.."),
			*FString(__FUNCTION__), __LINE__);
	}
}
#endif // WITH_EDITOR

