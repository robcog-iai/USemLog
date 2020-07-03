// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBaseIndividual.h"
#include "Individuals/SLIndividualComponent.h"
#include "GameFramework/Actor.h"
#include "Utils/SLTagIO.h"

// Ctor
USLBaseIndividual::USLBaseIndividual()
{
	ParentActor = nullptr;
	AttachedToActor = nullptr;
	AttachedToIndividual = nullptr;
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
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// Load semantic data
bool USLBaseIndividual::Load(bool bReset, bool bTryImport)
{
	if (bReset)
	{
		LoadReset();
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
	
	SetIsLoaded(LoadImpl(bTryImport));
	return IsLoaded();
}

// Save data to owners tag
bool USLBaseIndividual::ExportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Owner not set, cannot write to tags.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bMarkDirty = false;
	if (IsIdValueSet())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, TagTypeConst, "Id", Id, bOverwrite) || bMarkDirty;
	}
	if (IsClassValueSet())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, TagTypeConst, "Class", Class, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLBaseIndividual::ImportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's owner not set, cannot read from tags.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	bool bNewValue = false;
	bNewValue = ImportIdValue(bOverwrite) || bNewValue;
	bNewValue = ImportClassValue(bOverwrite) || bNewValue;
	return bNewValue;
}

// True if individual is part of another individual
bool USLBaseIndividual::IsAttachedToAnotherIndividual() const
{
	return AttachedToActor && AttachedToActor->IsValidLowLevel() && !AttachedToActor->IsPendingKill()
		&& AttachedToIndividual && AttachedToIndividual->IsValidLowLevel() && !AttachedToIndividual->IsPendingKill();
}

// Set the id value, if it is empty reset the individual as not loaded
void USLBaseIndividual::SetIdValue(const FString& NewId)
{
	if (!Id.Equals(NewId))
	{
		Id = NewId;
		OnNewIdValue.Broadcast(this, Id);
		if (!IsIdValueSet() && IsLoaded())
		{
			SetIsLoaded(false);
		}
		else if (IsIdValueSet() && !IsLoaded())
		{
			Load(false, false);
		}
	}
}

// Set the class value, if empty, reset the individual as not loaded
void USLBaseIndividual::SetClassValue(const FString& NewClass)
{
	if (!Class.Equals(NewClass))
	{
		Class = NewClass;
		OnNewClassValue.Broadcast(this, Class);
		if (!IsClassValueSet() && IsLoaded())
		{
			SetIsLoaded(false);
		}
		else if (IsClassValueSet() && !IsLoaded())
		{
			Load(false, false);
		}
	}
}

// Clear all values of the individual
void USLBaseIndividual::InitReset()
{
	LoadReset();
	ParentActor = nullptr;
	AttachedToActor = nullptr;
	AttachedToIndividual = nullptr;
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLBaseIndividual::LoadReset()
{
	SetIsLoaded(false);
	ClearIdValue();
	ClearClassValue();
}

// Clear any bound delegates (called when init is reset)
void USLBaseIndividual::ClearDelegateBounds()
{
	OnInitChanged.Clear();
	OnLoadedChanged.Clear();
	OnNewIdValue.Clear();
	OnNewClassValue.Clear();
}

// Set the init flag, broadcast on new value
void USLBaseIndividual::SetIsInit(bool bNewValue, bool bBroadcast)
{
	if (bIsInit != bNewValue)
	{
		if (!bNewValue && IsLoaded())
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

// Check that the parent actor is set and valid
bool USLBaseIndividual::HasValidParentActor() const
{
	return ParentActor && ParentActor->IsValidLowLevel() && !ParentActor->IsPendingKill();
}

// Set pointer to parent actor
bool USLBaseIndividual::SetParentActor()
{
	// First outer is the component, second the actor
	if (AActor* CompOwner = Cast<AActor>(GetOuter()->GetOuter()))
	{
		// Set the parent actor
		ParentActor = CompOwner;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s, could not acess parent actor.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Set attachment parent (part of individual)
bool USLBaseIndividual::SetAttachedToParent()
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Parent actor is not set %s, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	// Check if individual is part of another actor
	if (AActor* AttAct = ParentActor->GetAttachParentActor())
	{
		// Get the individual component of the actor
		if (UActorComponent* AC = AttAct->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			AttachedToActor = AttAct;
			AttachedToIndividual = CastChecked<USLIndividualComponent>(AC)->GetIndividualObject();
			return true;
		}

		// TODO check that the individual does not simulate physics (if it does the attachment breaks at runtime)
	}
	return false;
}

// Set references
bool USLBaseIndividual::InitImpl()
{
	if (HasValidParentActor() || SetParentActor())
	{
		SetAttachedToParent();
		return true;
	}
	return false;
}

// Set data
bool USLBaseIndividual::LoadImpl(bool bTryImport)
{
	bool bRetValue = true;
	if (!IsIdValueSet())		
	{
		if (bTryImport)
		{
			if (!ImportIdValue())
			{
				bRetValue = false;
			}
		}
		else
		{
			bRetValue = false;
		}
	}

	if (!IsClassValueSet())
	{
		if (bTryImport)
		{
			if (!ImportClassValue())
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
bool USLBaseIndividual::ImportIdValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsIdValueSet() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetIdValue(FSLTagIO::GetValue(ParentActor, TagTypeConst, "Id"));
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
bool USLBaseIndividual::ImportClassValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsClassValueSet() || bOverwrite)
	{
		const FString PrevVal = Class;
		SetClassValue(FSLTagIO::GetValue(ParentActor, TagTypeConst, "Class"));
		bNewValue = !Class.Equals(PrevVal);
		//if (!HasClass())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d No Class value could be imported from %s's tag.."),
		//		*FString(__FUNCTION__), __LINE__, *GetFullName());
		//}
	}
	return bNewValue;
}

