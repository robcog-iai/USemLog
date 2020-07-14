// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLBaseIndividual.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLBoneIndividual.h"
#include "Individuals/SLIndividualUtils.h"
#include "Skeletal/SLSkeletalDataAsset.h"

// Sets default values for this component's properties
USLIndividualComponent::USLIndividualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	bIsConnected = false;
	IndividualObj = nullptr;
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualComponent::OnRegister()
{
	Super::OnRegister();

	//if (!IsConnected())
	//{
	//	Connect();
	//}
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	// Check if actor already has a semantic data component
	for (const auto AC : GetOwner()->GetComponentsByClass(USLIndividualComponent::StaticClass()))
	{
		if (AC != this)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has an individual component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}
}

// Called before destroying the object.
void USLIndividualComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);
	OnDestroyed.Broadcast(this);

	if (HasValidIndividual())
	{
		IndividualObj->ConditionalBeginDestroy();
	}

	ClearDelegates();
	Super::BeginDestroy();
}

// Create and init individual
bool USLIndividualComponent::Init(bool bReset)
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

// Load individual
bool USLIndividualComponent::Load(bool bReset, bool bTryImport)
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

// Listen to the individual object delegates
bool USLIndividualComponent::Connect()
{
	if (IsConnected())
	{
		return true;
	}
	SetIsConnected(BindDelegates());
	return IsConnected();
}


/* Functionalities */
// Save data to owners tag
bool USLIndividualComponent::ExportValues(bool bOverwrite)
{
	if (HasValidIndividual())
	{
		return IndividualObj->ExportValues(bOverwrite);
	}
	return false;
}

// Load data from owners tag
bool USLIndividualComponent::ImportValues(bool bOverwrite)
{
	if (HasValidIndividual())
	{
		return IndividualObj->ImportValues(bOverwrite);
	}
	return false;
}

// Clear exported values
bool USLIndividualComponent::ClearExportedValues()
{
	if (HasValidIndividual())
	{
		return IndividualObj->ClearExportedValues();
	}
	return false;
}

// Toggle between original and mask material is possible
bool USLIndividualComponent::ToggleVisualMaskVisibility(bool bPrioritizeChildren)
{
	if (HasValidIndividual())
	{
		if (USLPerceivableIndividual* VI = Cast<USLPerceivableIndividual>(IndividualObj))
		{
			return VI->ToggleMaterials(bPrioritizeChildren);
		}
	}

	return false;
}

/* Values */
// Write new id value
bool USLIndividualComponent::WriteId(bool bOverwrite)
{
	if (HasValidIndividual())
	{
		if (!IndividualObj->IsIdValueSet() || bOverwrite)
		{
			IndividualObj->GenerateNewIdValue();
			return true;
		}
	}
	return false;
}

// Clear id value
bool USLIndividualComponent::ClearId()
{
	if (HasValidIndividual())
	{
		if (IndividualObj->IsIdValueSet())
		{
			IndividualObj->ClearIdValue();
			return true;
		}
	}
	return false;
}

// Write default class value
bool USLIndividualComponent::WriteClass(bool bOverwrite)
{
	if (HasValidIndividual())
	{
		if (!IndividualObj->IsClassValueSet() || bOverwrite)
		{
			IndividualObj->SetDefaultClassValue();
			return true;
		}
	}
	return false;
}

// Clear class value
bool USLIndividualComponent::ClearClass()
{
	if (HasValidIndividual())
	{
		if (IndividualObj->IsClassValueSet())
		{
			IndividualObj->ClearClassValue();
			return true;
		}
	}
	return false;
}

// Write visual mask value (if perceivable)
bool USLIndividualComponent::WriteVisualMaskClass(const FString& Value, bool bOverwrite, const TArray<FString>& ChildrenValues)
{
	if (HasValidIndividual())
	{
		if (USLPerceivableIndividual* VI = Cast<USLPerceivableIndividual>(IndividualObj))
		{
			if (!VI->IsVisualMaskValueSet() || bOverwrite)
			{
				VI->SetVisualMaskValue(Value);
			}

			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(VI))
			{
				if (ChildrenValues.Num() == SkI->GetBoneIndividuals().Num())
				{
					int32 Index = 0;
					for (const auto& BI : SkI->GetBoneIndividuals())
					{
						BI->SetVisualMaskValue(ChildrenValues[Index]);
						Index++;
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s's bones num differ of the values num, cannot set mask values.."),
						*FString(__FUNCTION__), __LINE__, *GetFullName());
				}

			}
			
			// TODO check robot type
			return true;
		}
	}
	return false;
}

// Clear visual masks
bool USLIndividualComponent::ClearVisualMask()
{
	if (HasValidIndividual())
	{
		bool RetVal = false;
		if (USLPerceivableIndividual* VI = Cast<USLPerceivableIndividual>(IndividualObj))
		{
			if (VI->IsVisualMaskValueSet())
			{
				VI->ClearVisualMaskValue();
				RetVal = true;
			}

			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(VI))
			{
				for (const auto& BI : SkI->GetBoneIndividuals())
				{
					BI->ClearVisualMaskValue();
					RetVal = true;
				}
			}
			return RetVal;
		}
	}
	return false;
}


// Clear all values of the individual
void USLIndividualComponent::InitReset()
{
	LoadReset();
	SetIsInit(false);
	if (HasValidIndividual())
	{
		IndividualObj->Init(true);
	}
	ClearDelegates();
}

// Clear all data of the individual
void USLIndividualComponent::LoadReset()
{
	SetIsLoaded(false);
	if (HasValidIndividual())
	{
		IndividualObj->Load(true);
	}
}

// Clear any bound delegates (called when init is reset)
void USLIndividualComponent::ClearDelegates()
{
	OnDestroyed.Clear();
	OnInitChanged.Clear();
	OnLoadedChanged.Clear();
	OnConnectedChanged.Clear();
	OnValueChanged.Clear();
	OnDelegatesCleared.Broadcast(this);
	OnDelegatesCleared.Clear();
}

// Set the init flag, broadcast on new value
void USLIndividualComponent::SetIsInit(bool bNewValue, bool bBroadcast)
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
void USLIndividualComponent::SetIsLoaded(bool bNewValue, bool bBroadcast)
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

// Set the connected flag, broadcast on new value
void USLIndividualComponent::SetIsConnected(bool bNewValue, bool bBroadcast)
{
	if (bIsConnected != bNewValue)
	{
		bIsConnected = bNewValue;
		if (bBroadcast)
		{
			OnConnectedChanged.Broadcast(this, bNewValue);
		}
	}
}

// Create individual if not created and forward init call
bool USLIndividualComponent::InitImpl()
{
	if (HasValidIndividual() || CreateIndividual())
	{
		Connect();
		return IndividualObj->Init();
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not create an individual, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Forward the laod call to the individual object
bool USLIndividualComponent::LoadImpl(bool bTryImport)
{
	if (HasValidIndividual())
	{
		return IndividualObj->Load(bTryImport);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no valid individual, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

// Sync states with the individual
bool USLIndividualComponent::BindDelegates()
{
	if (!HasValidIndividual())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual object is not valid, cannot bind delegates.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	// Bind init change delegate
	if (!IndividualObj->OnInitChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualInitChange))
	{
		IndividualObj->OnInitChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualInitChange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on init delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	// Bind load change delegate
	if (!IndividualObj->OnLoadedChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualLoadedChange))
	{
		IndividualObj->OnLoadedChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualLoadedChange);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on loaded delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	// Bind delegeates cleared 
	if (!IndividualObj->OnDelegatesCleared.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualDelegatesCleared))
	{
		IndividualObj->OnDelegatesCleared.AddDynamic(this, &USLIndividualComponent::OnIndividualDelegatesCleared);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on delegates cleared delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	/* Values delegates */
	// Id
	if (!IndividualObj->OnNewIdValue.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualNewId))
	{
		IndividualObj->OnNewIdValue.AddDynamic(this, &USLIndividualComponent::OnIndividualNewId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component new id delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	// Class
	if (!IndividualObj->OnNewClassValue.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualNewClass))
	{
		IndividualObj->OnNewClassValue.AddDynamic(this, &USLIndividualComponent::OnIndividualNewClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component new class delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	// Visual mask
	if (USLPerceivableIndividual* VI = Cast<USLPerceivableIndividual>(IndividualObj))
	{
		if (!VI->OnNewVisualMaskValue.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualNewVisualMask))
		{
			VI->OnNewVisualMaskValue.AddDynamic(this, &USLIndividualComponent::OnIndividualNewVisualMask);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component new visual mask delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}

	return true;
}

// Check if individual object is valid
bool USLIndividualComponent::HasValidIndividual() const
{
	return IndividualObj && IndividualObj->IsValidLowLevel() && !IndividualObj->IsPendingKill();
}

/* Private */
// Create the semantic individual
bool USLIndividualComponent::CreateIndividual()
{
	if (HasValidIndividual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	// Set semantic individual type depending on owner
	if (USLBaseIndividual* BI = FSLIndividualUtils::CreateIndividualObject(this, GetOwner()))
	{	
		IndividualObj = BI;

		// If skeletal, check for data asset
		if (BI->IsA(USLSkeletalIndividual::StaticClass()))
		{
			if (USLSkeletalDataAsset* SLSK = FSLIndividualUtils::FindSkeletalDataAsset(GetOwner()))
			{
				OptionalDataAssets.Add(SkelDataAssetKey, SLSK);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not find skeletal data asset for skeletal actor.."),
					*FString(__FUNCTION__), __LINE__);
			}
		}
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not create an individual object, component will self destruct.."),
			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		// Unknown individual type, destroy self
		ConditionalBeginDestroy();
		return false;
	}
}

// Triggered when the semantic individual init flag changes
void USLIndividualComponent::OnIndividualInitChange(USLBaseIndividual* Individual, bool bNewValue)
{
	SetIsInit(bNewValue);
}

// Triggered when the semantic individual loaded flag changes
void USLIndividualComponent::OnIndividualLoadedChange(USLBaseIndividual* Individual, bool bNewValue)
{
	SetIsLoaded(bNewValue);
}

// Triggered on individual id change
void USLIndividualComponent::OnIndividualNewId(USLBaseIndividual* Individual, const FString& NewId)
{
	OnValueChanged.Broadcast(this, "Id", NewId);
}

// Triggered on individual class change
void  USLIndividualComponent::OnIndividualNewClass(USLBaseIndividual* Individual, const FString& NewClass)
{
	OnValueChanged.Broadcast(this, "Class", NewClass);
}

// Triggered on individual visual mask change
void  USLIndividualComponent::OnIndividualNewVisualMask(USLBaseIndividual* Individual, const FString& NewVisualMask)
{
	OnValueChanged.Broadcast(this, "VisualMask", NewVisualMask);
}

// Triggered with the delegates are cleared on the individual object (required reconnection afterwards)
void USLIndividualComponent::OnIndividualDelegatesCleared(USLBaseIndividual* Individual)
{
	SetIsConnected(false);
}

