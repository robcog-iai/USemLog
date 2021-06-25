// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLBaseIndividual.h"
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
	bIsMovable = true;
	TagType = "SemLog";

	/* SemLog World state logger workaround helper */
	bHasMovedFlag = false;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLBaseIndividual::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLBaseIndividual, bReloadParentButton))
	{
		bReloadParentButton = false;
		SetParentActor();
	}
}
#endif // WITH_EDITOR

// Called before destroying the object.
void USLBaseIndividual::BeginDestroy()
{
	SetIsInit(false);
	ClearDelegates();
	Super::BeginDestroy();
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

// Trigger values as new value broadcast
void USLBaseIndividual::TriggerValuesBroadcast()
{
	OnNewValue.Broadcast(this, "Id", Id);
	OnNewValue.Broadcast(this, "Class", Class);
	for (const auto& CI : GetChildrenIndividuals())
	{
		CI->TriggerValuesBroadcast();
	}
}

// Save data to owners tag
bool USLBaseIndividual::ExportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No valid parent actor found, could not export values"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bNewValue = false;
	if (IsIdValueSet() && FSLTagIO::AddKVPair(ParentActor, TagType, "Id", Id, bOverwrite))
	{
		bNewValue = true;
	}
	if (IsClassValueSet() && FSLTagIO::AddKVPair(ParentActor, TagType, "Class", Class, bOverwrite))
	{
		bNewValue = true;
	}
	//if (IsOIdValueSet() && FSLTagIO::AddKVPair(ParentActor, ImportTagType, "OId", OId, bOverwrite))
	//{
	//	bNewValue = true;
	//}
	for (const auto& CI : GetChildrenIndividuals())
	{
		if (CI->ExportValues(bOverwrite))
		{
			bNewValue = true;
		}
	}
	return bNewValue;
}

// Load data from owners tag
bool USLBaseIndividual::ImportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No valid parent actor found, could not import values"), *FString(__FUNCTION__), __LINE__);
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
	//if (ImportOIdValue(bOverwrite))
	//{
	//	bNewValue = true;
	//}
	for (const auto& CI : GetChildrenIndividuals())
	{
		if (CI->ImportValues(bOverwrite))
		{
			bNewValue = true;
		}
	}
	return bNewValue;
}

// Clear exported values
bool USLBaseIndividual::ClearExportedValues()
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No valid parent actor found, could not clear exported values"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bNewValue = false;
	int32 TagIndex = INDEX_NONE;
	if (FSLTagIO::HasType(ParentActor, TagType, &TagIndex))
	{
		ParentActor->Tags.RemoveAt(TagIndex);
		bNewValue = true;
	}

	for (const auto& CI : GetChildrenIndividuals())
	{
		if (CI->ClearExportedValues())
		{
			bNewValue = true;
		}
	}
	return bNewValue;
}

// Cache the current transform of the individual (returns true on a new value)
bool USLBaseIndividual::UpdateCachedPose(float Tolerance, FTransform* OutPose)
{
	if (IsInit())
	{
		const FTransform CurrPose = ParentActor->GetTransform();
		if(!CachedPose.Equals(CurrPose, Tolerance))
		{
			CachedPose = CurrPose;
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return true;
		}
		else
		{
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return false;
		}
	}
	else
	{
		if (!CachedPose.Equals(FTransform::Identity, Tolerance))
		{
			CachedPose = FTransform::Identity;
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return true;
		}
		else
		{
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return false;
		}
	}	
}

// True if individual is part of another individual
bool USLBaseIndividual::IsAttachedToAnotherIndividual() const
{
	return AttachedToActor && AttachedToActor->IsValidLowLevel() && !AttachedToActor->IsPendingKill()
		&& AttachedToIndividual && AttachedToIndividual->IsValidLowLevel() && !AttachedToIndividual->IsPendingKill();
}

// Get all children of the individual in a newly created array
const TArray<USLBaseIndividual*> USLBaseIndividual::GetChildrenIndividuals() const
{
	return TArray<USLBaseIndividual*>();
}

// Get info about the individual
FString USLBaseIndividual::GetInfo() const
{
	FString Info;
	Info.Append(FString::Printf(TEXT("ParentActor=%s; "), ParentActor ? *ParentActor->GetName() : *FString("null")));
	//Info.Append(FString::Printf(TEXT("AttachedToActor=%s; "), AttachedToActor ? *AttachedToActor->GetName() : *FString("null")));
	Info.Append(FString::Printf(TEXT("Id=%s; "), *Id));
	Info.Append(FString::Printf(TEXT("Class=%s; "), *Class));
	return Info;
}

// Generate a new id for the individual
FString USLBaseIndividual::GenerateNewId() const
{
	return FSLUuid::NewGuidInBase64Url();
}

// Generate class name, virtual since each invidiual type will have different name
FString USLBaseIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

/* Id */
// Set the id value, if it is empty reset the individual as not loaded
void USLBaseIndividual::SetIdValue(const FString& NewVal)
{
	if (!Id.Equals(NewVal))
	{
		Id = NewVal;
		OnNewValue.Broadcast(this, "Id", Id);
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

// Generate a new unique id value
void USLBaseIndividual::GenerateNewIdValue()
{
	SetIdValue(FSLUuid::NewGuidInBase64Url());
}

// Set the class value, if empty, reset the individual as not loaded
void USLBaseIndividual::SetClassValue(const FString& NewVal)
{
	if (!Class.Equals(NewVal))
	{
		Class = NewVal;
		OnNewValue.Broadcast(this, "Class", Class);
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

// Set the default class name to the individual
void USLBaseIndividual::SetDefaultClassValue()
{
	SetClassValue(CalcDefaultClassValue());
}

//// Set the bson id value from string, does not check for validity
//void USLBaseIndividual::SetOIdValue(const FString& NewVal)
//{
//	if (!OId.Equals(NewVal))
//	{
//		OId = NewVal;
//	}
//}

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
		if (!ParentActor->IsRootComponentMovable())
		{
			bIsMovable = false;
		}
		//SetAttachedToParent();
		return true;
	}
	return false;
}

// Set data
bool USLBaseIndividual::LoadImpl(bool bTryImport)
{
	// Explicitly check later to avoid having the attached to parent being init after itself
	SetAttachedToParent();
	UpdateCachedPose();

	bool bRetValue = true;
	if (!IsIdValueSet())		
	{
		if (bTryImport)
		{
			if (!ImportIdValue())
			{
				GenerateNewIdValue();
			}
		}
	}

	if (!IsClassValueSet())
	{
		if (bTryImport)
		{
			if (!ImportClassValue())
			{
				SetDefaultClassValue();
			}
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

// Clear all values of the individual
void USLBaseIndividual::InitReset()
{
	LoadReset();
	ParentActor = nullptr;
	AttachedToActor = nullptr;
	AttachedToIndividual = nullptr;
	SetIsInit(false);
	ClearDelegates();
}

// Clear all data of the individual
void USLBaseIndividual::LoadReset()
{
	SetIsLoaded(false);
	ClearIdValue();
	ClearClassValue();
}

// Clear any bound delegates (called when init is reset)
void USLBaseIndividual::ClearDelegates()
{
	OnInitChanged.Clear();
	OnLoadedChanged.Clear();
	OnNewValue.Clear();
	OnDelegatesCleared.Broadcast(this);
	OnDelegatesCleared.Clear();
}

// Import id from tag, true if new value is written
bool USLBaseIndividual::ImportIdValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsIdValueSet() || bOverwrite)
	{
		const FString PrevVal = Id;
		SetIdValue(FSLTagIO::GetValue(ParentActor, TagType, "Id"));
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
		SetClassValue(FSLTagIO::GetValue(ParentActor, TagType, "Class"));
		bNewValue = !Class.Equals(PrevVal);
	}
	return bNewValue;
}

//// Import the oid value
//bool USLBaseIndividual::ImportOIdValue(bool bOverwrite)
//{
//	bool bNewValue = false;
//	if (!IsOIdValueSet() || bOverwrite)
//	{
//		const FString PrevVal = OId;
//		SetIdValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "OId"));
//		bNewValue = !OId.Equals(PrevVal);
//	}
//	return bNewValue;
//}
//
//// Load the oid value from the persitent OId string (generate a new one if none is available)
//bool USLBaseIndividual::LoadOId(bool bGenerateNew)
//{
//#if SL_WITH_LIBMONGO_C
//	if (IsOIdValueSet())
//	{		
//		const char* oid_str = TCHAR_TO_ANSI(*OId);//TCHAR_TO_UTF8
//		if (bson_oid_is_valid(oid_str, sizeof oid_str))
//		{
//			bson_oid_init_from_string(&oid, oid_str);
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's oid (%s) succefully loaded.."),
//				*FString(__FUNCTION__), __LINE__, *GetFullName(), ANSI_TO_TCHAR(oid_str));
//			return true;
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's new oid string value (%s) not valid.."),
//				*FString(__FUNCTION__), __LINE__, *GetFullName(), ANSI_TO_TCHAR(oid_str));
//			return false;
//		}
//	}
//	else if (bGenerateNew)
//	{
//		bson_oid_init(&oid, NULL);
//		char oid_str[25];
//		bson_oid_to_string(&oid, oid_str);
//		SetOIdValue( UTF8_TO_TCHAR(oid_str));
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's oid (%s) was succefully generated.."),
//			*FString(__FUNCTION__), __LINE__, *GetFullName(), *OId);
//		return true;
//	}
//	return false;
//	UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no oid.."), *FString(__FUNCTION__), __LINE__, *GetFullName());
//#else
//	return false;
//#endif
//}

