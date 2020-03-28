// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividual.h"
#include "Data/SLSkeletalIndividual.h"
#include "Data/SLVisualIndividual.h"
#include "ScopedTransaction.h"

#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

// Sets default values for this component's properties
USLIndividualComponent::USLIndividualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

	//// Check if actor already has a semantic data component
	//for (const auto AC : Owner->GetComponentsByClass(USLIndividualComponent::StaticClass()))
	//{
	//	if (AC != this)
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has a semantic data component:%s, self-destruction commenced.."),
	//			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
	//		DestroyComponent();
	//		return;
	//	}
	//}

	// Set semantic individual type depending on owner
	if (Owner->IsA(AStaticMeshActor::StaticClass()))
	{
		ConvertToSemanticIndividual = USLVisualIndividual::StaticClass();
		SemanticIndividualObject = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
	}
	else if (Owner->IsA(ASkeletalMeshActor::StaticClass()))
	{
		ConvertToSemanticIndividual = USLSkeletalIndividual::StaticClass();
		SemanticIndividualObject = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d unsuported actor type for generating a semantic individual component %s.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		ConditionalBeginDestroy();
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t Created component .... "), *FString(__FUNCTION__), __LINE__);
}

// Called when the game starts
void USLIndividualComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("%s::%d ******** %s .... "), *FString(__FUNCTION__), __LINE__, *GetName());
	ToString();
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
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, ConvertToSemanticIndividual))
	{
		DoConvertDataType();
	}
}

FString USLIndividualComponent::ToString() const
{
	for (TFieldIterator<UProperty> It(GetClass()); It; ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_Transient))
		{
			continue;
		}

	

		UE_LOG(LogTemp, Warning, TEXT("%s::%d UP=%s"), *FString(__FUNCTION__), __LINE__, *It->GetName());
		//It->HasMetaData
	}
	return FString();
}

// Convert datat type object to the selected class type
void USLIndividualComponent::DoConvertDataType()
{
	if (ConvertToSemanticIndividual)
	{		
		if (SemanticIndividualObject && !SemanticIndividualObject->IsPendingKill())
		{
			if(SemanticIndividualObject->StaticClass() != ConvertToSemanticIndividual)
			{
				SemanticIndividualObject->ConditionalBeginDestroy();
				SemanticIndividualObject = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Converted datatype to %s (%s).."),
					*FString(__FUNCTION__), __LINE__, *ConvertToSemanticIndividual->GetName(), *SemanticIndividualObject->StaticClass()->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Current datatype %s is of the same type as %s.."),
					*FString(__FUNCTION__), __LINE__, *SemanticIndividualObject->StaticClass()->GetName(), *ConvertToSemanticIndividual->GetName());
			}
		}
		else
		{
			SemanticIndividualObject = NewObject<USLIndividualBase>(this, ConvertToSemanticIndividual);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Creating new %s datatype (%s).."),
				*FString(__FUNCTION__), __LINE__, *ConvertToSemanticIndividual->GetName(), *SemanticIndividualObject->StaticClass()->GetName());
		}
	}
	else if (SemanticIndividualObject && !SemanticIndividualObject->IsPendingKill())
	{
		SemanticIndividualObject->ConditionalBeginDestroy();
		SemanticIndividualObject = nullptr;
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

