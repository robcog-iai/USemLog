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
}

// Set pointer to the semantic owner
bool USLBaseIndividual::Init(bool bReset)
{
	if (bReset)
	{
		SetIsInit(false);
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// Load semantic data
bool USLBaseIndividual::Load(bool bReset)
{
	if (bReset)
	{
		SetIsLoaded(false);
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
	
	SetIsLoaded(LoadImpl());
	return IsLoaded();
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
	if (HasId())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "Id", Id, bOverwrite) || bMarkDirty;
	}
	if (HasClass())
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

// Set the id value, if it is empty reset the individual as not loaded
void USLBaseIndividual::SetId(const FString& NewId)
{
	if (!Id.Equals(NewId))
	{
		Id = NewId;
		OnNewIdValue.Broadcast(this, Id);
		SetIsLoaded(LoadImpl(false));
	}
}

// Set the class value, if empty, reset the individual as not loaded
void USLBaseIndividual::SetClass(const FString& NewClass)
{
	if (!Class.Equals(NewClass))
	{
		Class = NewClass;
		OnNewClassValue.Broadcast(this, Class);
		SetIsLoaded(LoadImpl(false));
	}
}

// Set the init flag, broadcast on new value
void USLBaseIndividual::SetIsInit(bool bNewValue)
{
	if (bIsInit != bNewValue)
	{
		bIsInit = bNewValue;
		OnInitChanged.Broadcast(this, bNewValue);
	}
}

// Set the loaded flag, broadcast on new value
void USLBaseIndividual::SetIsLoaded(bool bNewValue)
{
	if (bIsLoaded != bNewValue)
	{
		bIsLoaded = bNewValue;
		OnLoadedChanged.Broadcast(this, bNewValue);
	}
}


// Import id from tag, true if new value is written
bool USLBaseIndividual::ImportIdFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (!HasId() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetId(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Id"));
		bNewValue = !Id.Equals(PrevVal);
		//if (!HasId())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d No Id value could be imported from %s's tag.."),
		//		*FString(__FUNCTION__), __LINE__, *GetFullName());
		//}
	}
	return bNewValue;
}

// Import class from tag, true if new value is written
bool USLBaseIndividual::ImportClassFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (!HasClass() || bOverwrite)
	{
		const FString PrevVal = Class;
		SetClass(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Class"));
		bNewValue = !Class.Equals(PrevVal);
		//if (!HasClass())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d No Class value could be imported from %s's tag.."),
		//		*FString(__FUNCTION__), __LINE__, *GetFullName());
		//}
	}
	return bNewValue;
}

// Private init implementation
bool USLBaseIndividual::InitImpl()
{
	// First outer is the component, second the actor
	if (AActor* CompOwner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		SemanticOwner = CompOwner;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s, has no semantic owner.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Private load implementation
bool USLBaseIndividual::LoadImpl(bool bTryImportFromTags)
{
	bool bSuccess = true;
	if (!HasId())		
	{
		if (bTryImportFromTags)
		{
			if (!ImportIdFromTag())
			{
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}

	if (!HasClass())
	{
		if (bTryImportFromTags)
		{
			if (!ImportClassFromTag())
			{
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}
	return bSuccess;
}


