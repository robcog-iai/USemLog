// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualBase.h"

// Ctor
USLIndividualBase::USLIndividualBase()
{
	bIsInit = false;
}

// Set the semantic owner actor
void USLIndividualBase::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLIndividualBase::Init()
{
	if (bIsInit)
	{
		return true;
	}

	// First outer is the component, second the actor
	if (AActor* Owner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		SetSemanticOwner(Owner);
		bIsInit = true;
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not init %s.."), *FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}


// Save data to owners tag
bool USLIndividualBase::SaveToTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot write to tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	return true;
}

// Load data from owners tag
bool USLIndividualBase::LoadFromTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot read from tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	return true;
}

// All properties are set for runtime
bool USLIndividualBase::IsRuntimeReady() const
{
	return SemanticOwner != nullptr;
}

