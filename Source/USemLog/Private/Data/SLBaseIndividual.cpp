// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLBaseIndividual.h"
#include "Data/SLIndividualComponent.h"
#include "GameFramework/Actor.h"
#include "Utils/SLTagIO.h"

// Ctor
USLBaseIndividual::USLBaseIndividual()
{
	ParentActor = nullptr;
	PartOfActor = nullptr;
	PartOfIndividual = nullptr;
	bIsInit = false;
	bIsLoaded = false;
}

// Called before destroying the object.
void USLBaseIndividual::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	Super::BeginDestroy();
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
		SetIsInit(false, false);
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// Load semantic data
bool USLBaseIndividual::Load(bool bReset, bool bTryImportFromTags)
{
	if (bReset)
	{
		SetIsLoaded(false, false);
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
	
	SetIsLoaded(LoadImpl(bTryImportFromTags));
	return IsLoaded();
}

// Save data to owners tag
bool USLBaseIndividual::ExportToTag(bool bOverwrite)
{
	if (!ParentActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot write to tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bMarkDirty = false;
	if (HasId())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, TagTypeConst, "Id", Id, bOverwrite) || bMarkDirty;
	}
	if (HasClass())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, TagTypeConst, "Class", Class, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLBaseIndividual::ImportFromTag(bool bOverwrite)
{
	if (!ParentActor)
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
		if (!HasId() && IsLoaded())
		{
			SetIsLoaded(false);
		}
		else if (HasId() && !IsLoaded())
		{
			// Check if the individual can now be loaded
			Load(false, false);
		}
	}
}

// Set the class value, if empty, reset the individual as not loaded
void USLBaseIndividual::SetClass(const FString& NewClass)
{
	if (!Class.Equals(NewClass))
	{
		Class = NewClass;
		OnNewClassValue.Broadcast(this, Class);
		if (!HasClass() && IsLoaded())
		{
			SetIsLoaded(false);
		}
		else if (HasClass() && !IsLoaded())
		{
			// Check if the individual can now be loaded
			Load(false, false);
		}
	}
}

// Set the init flag, broadcast on new value
void USLBaseIndividual::SetIsInit(bool bNewValue, bool bBroadcast)
{
	if (bIsInit != bNewValue)
	{
		if (!bNewValue)
		{
			SetIsLoaded(false);
		}

		bIsInit = bNewValue;
		if (bBroadcast)
		{
			OnInitChanged.Broadcast(this, bNewValue);
		}
	}
}

// Set the loaded flag, broadcast on new value
void USLBaseIndividual::SetIsLoaded(bool bNewValue, bool bBroadcast)
{
	if (bIsLoaded != bNewValue)
	{
		bIsLoaded = bNewValue;
		if (bBroadcast)
		{
			OnLoadedChanged.Broadcast(this, bNewValue);
		}
	}
}

// Private init implementation
bool USLBaseIndividual::InitImpl()
{
	// First outer is the component, second the actor
	if (AActor* CompOwner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		// Set the parent actor
		ParentActor = CompOwner;

		// Check if individual is part of another actor
		if (AActor* AttAct = CompOwner->GetAttachParentActor())
		{
			// Get the individual component of the actor
			if (UActorComponent* AC = AttAct->GetComponentByClass(USLIndividualComponent::StaticClass()))
			{
				PartOfActor = AttAct;
				PartOfIndividual = CastChecked<USLIndividualComponent>(AC)->GetIndividualObject();
			}

			// TODO check that the individual does not simulate physics (if it does the attachment breaks at runtime)
		}

		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s, has no semantic owner.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Private load implementation
bool USLBaseIndividual::LoadImpl(bool bTryImportFromTags)
{
	bool bRetValue = true;
	if (!HasId())		
	{
		if (bTryImportFromTags)
		{
			if (!ImportIdFromTag())
			{
				bRetValue = false;
			}
		}
		else
		{
			bRetValue = false;
		}
	}

	if (!HasClass())
	{
		if (bTryImportFromTags)
		{
			if (!ImportClassFromTag())
			{
				bRetValue = false;
			}
		}
		else
		{
			bRetValue = false;
		}
	}
	return bRetValue;
}

// Import id from tag, true if new value is written
bool USLBaseIndividual::ImportIdFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (!HasId() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetId(FSLTagIO::GetValue(ParentActor, TagTypeConst, "Id"));
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
		SetClass(FSLTagIO::GetValue(ParentActor, TagTypeConst, "Class"));
		bNewValue = !Class.Equals(PrevVal);
		//if (!HasClass())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d No Class value could be imported from %s's tag.."),
		//		*FString(__FUNCTION__), __LINE__, *GetFullName());
		//}
	}
	return bNewValue;
}

