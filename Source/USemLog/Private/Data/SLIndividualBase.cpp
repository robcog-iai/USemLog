// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualBase.h"
#include "GameFramework/Actor.h"

// Ctor
USLIndividualBase::USLIndividualBase()
{
	bIsInitPrivate = false;
	bIsLoadedPrivate = false;
}

// Set the semantic owner actor
void USLIndividualBase::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLIndividualBase::Init(bool bForced)
{
	if (bForced)
	{
		bIsInitPrivate = false;
	}

	if (IsInit())
	{
		return true;
	}

	bIsInitPrivate = InitImpl();
	return bIsInitPrivate;
}

// Check if individual is initialized
bool USLIndividualBase::IsInit() const
{
	return bIsInitPrivate;
}

// Load semantic data
bool USLIndividualBase::Load(bool bForced)
{
	if (bForced)
	{
		bIsLoadedPrivate = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		return false;
	}
	
	bIsLoadedPrivate = LoadImpl();
	return bIsLoadedPrivate;
}

// Check if semantic data is succesfully loaded
bool USLIndividualBase::IsLoaded() const
{
	return bIsLoadedPrivate;
}

// Save data to owners tag
bool USLIndividualBase::ExportToTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot write to tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	return true;
}

// Load data from owners tag
bool USLIndividualBase::ImportFromTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot read from tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	return true;
}

// Private init implementation
bool USLIndividualBase::InitImpl()
{
	SemanticOwner = nullptr;
	// First outer is the component, second the actor
	if (AActor* Owner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		SemanticOwner = Owner;
	}
	return SemanticOwner->IsValidLowLevel();
}

bool USLIndividualBase::LoadImpl()
{
	// Nothing to load for runtime
	return true;
}


