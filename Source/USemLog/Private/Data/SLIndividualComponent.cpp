// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividual.h"
#include "Data/SLSkeletalIndividual.h"
#include "Data/SLVisualIndividual.h"

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
#endif // WITH_EDITOR

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

