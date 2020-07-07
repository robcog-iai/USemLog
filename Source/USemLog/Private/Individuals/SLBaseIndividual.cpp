// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBaseIndividual.h"
#include "Individuals/SLIndividualComponent.h"
#include "GameFramework/Actor.h"

// Utils
#include "Utils/SLTagIO.h"
#include "Utils/SLUuid.h"


// Ctor
USLBaseIndividual::USLBaseIndividual()
{
	ParentActor = nullptr;
	AttachedToActor = nullptr;
	AttachedToIndividual = nullptr;
	bIsInit = false;
	bIsLoaded = false;
	ImportTagType = "SemLog";
}

// Called before destroying the object.
void USLBaseIndividual::BeginDestroy()
{
	SetIsInit(false);
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
	if (!IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init, cannot export values.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	bool bNewValue = false;
	if (IsIdValueSet() && FSLTagIO::AddKVPair(ParentActor, ImportTagType, "Id", Id, bOverwrite))
	{
		bNewValue = true;
	}
	if (IsClassValueSet() && FSLTagIO::AddKVPair(ParentActor, ImportTagType, "Class", Class, bOverwrite))
	{
		bNewValue = true;
	}
	if (IsOIdValueSet() && FSLTagIO::AddKVPair(ParentActor, ImportTagType, "OId", OId, bOverwrite))
	{
		bNewValue = true;
	}
	return bNewValue;
}

// Load data from owners tag
bool USLBaseIndividual::ImportValues(bool bOverwrite)
{
	if (!IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init, cannot import values.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	bool bNewValue = false;
	if (ImportIdValue(bOverwrite))
	{
		bNewValue = true;
	}
	if (ImportClassValue(bOverwrite))
	{
		bNewValue = true;
	}
	if (ImportOIdValue(bOverwrite))
	{
		bNewValue = true;
	}
	return bNewValue;
}

// Clear exported values
bool USLBaseIndividual::ClearExportedValues()
{
	if (!IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init, cannot remove any exported values values.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	int32 Pos = INDEX_NONE;
	if (FSLTagIO::HasType(ParentActor, ImportTagType, &Pos))
	{
		ParentActor->Tags.RemoveAt(Pos);
		return true;
	}	
	return false;
}

// True if individual is part of another individual
bool USLBaseIndividual::IsAttachedToAnotherIndividual() const
{
	return AttachedToActor && AttachedToActor->IsValidLowLevel() && !AttachedToActor->IsPendingKill()
		&& AttachedToIndividual && AttachedToIndividual->IsValidLowLevel() && !AttachedToIndividual->IsPendingKill();
}

// Generate a new id for the individual
FString USLBaseIndividual::GenerateNewId() const
{
	return FSLUuid::NewGuidInBase64Url();
}

// Generate class name, virtual since each invidiual type will have different name
FString USLBaseIndividual::GetClassName() const
{
	return GetTypeName();
}

// Set the id value, if it is empty reset the individual as not loaded
void USLBaseIndividual::SetIdValue(const FString& NewVal)
{
	if (!Id.Equals(NewVal))
	{
		Id = NewVal;
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
void USLBaseIndividual::SetClassValue(const FString& NewVal)
{
	if (!Class.Equals(NewVal))
	{
		Class = NewVal;
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

// Set the bson id value from string, does not check for validity
void USLBaseIndividual::SetOIdValue(const FString& NewVal)
{
	if (!OId.Equals(NewVal))
	{
		OId = NewVal;
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
	if (UActorComponent* AC = Cast<UActorComponent>(GetOuter()))
	{
		if (AActor* CompOwner = Cast<AActor>(AC->GetOuter()))
		{
			ParentActor = CompOwner;
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's second outer should be the parent actor.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer should be an actor component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	//// First outer is the component, second the actor
	//if (AActor* CompOwner = Cast<AActor>(GetOuter()->GetOuter()))
	//{
	//	// Set the parent actor
	//	ParentActor = CompOwner;
	//	return true;
	//}
	//UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s, could not acess parent actor.."),
	//	*FString(__FUNCTION__), __LINE__, *GetFullName());
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
				SetIdValue(GenerateNewId());
			}
		}
		else
		{
			SetIdValue(FSLUuid::NewGuidInBase64Url());
		}
	}

	if (!IsClassValueSet())
	{
		if (bTryImport)
		{
			if (!ImportClassValue())
			{
				SetClassValue(GetClassName());
			}
		}
		else
		{
			SetClassValue(GetClassName());
		}
	}

	//// Does not influence the load status, oid only required at runtime
	//if (!IsOIdValueSet())
	//{
	//	if (bTryImport)
	//	{
	//		ImportOIdValue();
	//	}
	//}

	return IsIdValueSet() && IsClassValueSet();
}

// Import id from tag, true if new value is written
bool USLBaseIndividual::ImportIdValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsIdValueSet() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetIdValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "Id"));
		bNewValue = !Id.Equals(PrevVal);
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
		SetClassValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "Class"));
		bNewValue = !Class.Equals(PrevVal);
	}
	return bNewValue;
}

// Import the oid value
bool USLBaseIndividual::ImportOIdValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsOIdValueSet() || bOverwrite)
	{
		const FString PrevVal = OId;
		SetIdValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "OId"));
		bNewValue = !OId.Equals(PrevVal);
	}
	return bNewValue;
}

// Load the oid value from the persitent OId string (generate a new one if none is available)
bool USLBaseIndividual::LoadOId(bool bGenerateNew)
{
#if SL_WITH_LIBMONGO_C
	if (IsOIdValueSet())
	{		
		const char* oid_str = TCHAR_TO_ANSI(*OId);//TCHAR_TO_UTF8
		if (bson_oid_is_valid(oid_str, sizeof oid_str))
		{
			bson_oid_init_from_string(&oid, oid_str);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's oid (%s) succefully loaded.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), ANSI_TO_TCHAR(oid_str));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's new oid string value (%s) not valid.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), ANSI_TO_TCHAR(oid_str));
			return false;
		}
	}
	else if (bGenerateNew)
	{
		bson_oid_init(&oid, NULL);
		char oid_str[25];
		bson_oid_to_string(&oid, oid_str);
		SetOIdValue( UTF8_TO_TCHAR(oid_str));
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's oid (%s) was succefully generated.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName(), *OId);
		return true;
	}
	return false;
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no oid.."), *FString(__FUNCTION__), __LINE__, *GetFullName());
#elif
	return false;
#endif
}

