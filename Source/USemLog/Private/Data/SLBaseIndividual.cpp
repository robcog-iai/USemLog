// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLBaseIndividual.h"
#include "GameFramework/Actor.h"
#include "Utils/SLTagIO.h"

// Ctor
USLBaseIndividual::USLBaseIndividual()
{
	SemanticOwner = nullptr;
	bIsInit = false;
	bIsLoaded = false;
}

// Set the semantic owner actor
void USLBaseIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLBaseIndividual::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
	}

	if (IsInit())
	{
		return true;
	}

	bIsInit = InitImpl();
	return bIsInit;
}

// Check if individual is initialized
bool USLBaseIndividual::IsInit() const
{
	return bIsInit;
}

// Load semantic data
bool USLBaseIndividual::Load(bool bReset)
{
	if (bReset)
	{
		bIsLoaded = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			return false;
		}
	}
	
	bIsLoaded = LoadImpl();
	return bIsLoaded;
}

// Check if semantic data is succesfully loaded
bool USLBaseIndividual::IsLoaded() const
{
	return bIsLoaded;
}

// Save data to owners tag
bool USLBaseIndividual::ExportToTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot write to tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bMarkDirty = false;
	if (!Id.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "Id", Id, bOverwrite) || bMarkDirty;
	}
	if (!Class.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "Class", Class, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLBaseIndividual::ImportFromTag(bool bOverwrite)
{
	if (!SemanticOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's owner not set, cannot read from tags.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	bool bNewValue = false;
	bNewValue = ImportIdFromTag(bOverwrite) || bNewValue;
	bNewValue = ImportClassFromTag(bOverwrite) || bNewValue;
	return bNewValue;
}

// Import id from tag, true if new value is written
bool USLBaseIndividual::ImportIdFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (Id.IsEmpty() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetId(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Id"));
		bNewValue = Id.Equals(PrevVal);
		if (Id.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No Id value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Import class from tag, true if new value is written
bool USLBaseIndividual::ImportClassFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (Class.IsEmpty() || bOverwrite)
	{
		const FString PrevVal = Class;
		SetClass(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Class"));
		bNewValue = Class.Equals(PrevVal);
		if (Class.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No Class value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return false;
}

// Private init implementation
bool USLBaseIndividual::InitImpl()
{
	SemanticOwner = nullptr;
	// First outer is the component, second the actor
	if (AActor* Owner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		SemanticOwner = Owner;
	}
	return SemanticOwner->IsValidLowLevel();
}

bool USLBaseIndividual::LoadImpl()
{
	bool bSuccess = true;
	if (!HasId())
	{
		if (!ImportIdFromTag())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no Id, tag import failed as well.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bSuccess = false;
		}
	}

	if (!HasClass())
	{
		if (!ImportClassFromTag())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no Class, tag import failed as well.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			bSuccess = false;
		}
	}
	
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's load failed.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	return bSuccess;
}


