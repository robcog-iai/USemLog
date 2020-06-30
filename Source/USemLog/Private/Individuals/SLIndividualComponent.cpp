// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Individuals/SLPerceivableIndividual.h"
#include "Individuals/SLIndividualUtils.h"

// Sets default values for this component's properties
USLIndividualComponent::USLIndividualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	IndividualObj = nullptr;
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualComponent::OnRegister()
{
	Super::OnRegister();

	// Re-bind delegates if the init state 
	if (IsInit())
	{
		BindDelegates();
	}
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
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has a semantic data component (%s), self-destruction commenced.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
			//DestroyComponent();
			ConditionalBeginDestroy();
			return;
		}
	}

	CreateIndividual();
}

// Called before destroying the object.
void USLIndividualComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	OnDestroyed.Broadcast(this);

	if (HasIndividual())
	{
		IndividualObj->ConditionalBeginDestroy();
	}

	Super::BeginDestroy();
}


// Set owner and individual
bool USLIndividualComponent::Init(bool bReset)
{
	if (bReset)
	{
		SetIsInit(false, false);
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl(bReset));
	return IsInit();
}

// Load individual
bool USLIndividualComponent::Load(bool bReset, bool bTryImportFromTags)
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

	SetIsLoaded(LoadImpl(bReset, bTryImportFromTags));
	return IsLoaded();
}


/* Functionalities */
// Save data to owners tag
bool USLIndividualComponent::ExportToTag(bool bOverwrite)
{
	if (IndividualObj && IndividualObj->IsValidLowLevel())
	{
		return IndividualObj->ExportToTag(bOverwrite);
	}
	return false;
}

// Load data from owners tag
bool USLIndividualComponent::ImportFromTag(bool bOverwrite)
{
	if (IndividualObj && IndividualObj->IsValidLowLevel())
	{
		return IndividualObj->ImportFromTag(bOverwrite);
	}
	return false;
}

// Toggle between original and mask material is possible
bool USLIndividualComponent::ToggleVisualMaskVisibility()
{
	if (USLPerceivableIndividual* SI = GetCastedIndividualObject<USLPerceivableIndividual>())
	{
		return SI->ToggleMaterials();
	}
	return false;
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

// Create individual if not created and forward init call
bool USLIndividualComponent::InitImpl(bool bReset)
{
	if (HasIndividual() || CreateIndividual())
	{
		return IndividualObj->Init(bReset);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s Could not create individual.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	return false;
}

// Forward the laod call to the individual object
bool USLIndividualComponent::LoadImpl(bool bReset, bool bTryImportFromTags)
{
	if (HasIndividual())
	{
		return IndividualObj->Load(bReset, bTryImportFromTags);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s This should not happen, and idividual should be created here.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	return false;
}

// Sync states with the individual
bool USLIndividualComponent::BindDelegates()
{
	if (HasIndividual())
	{
		// Bind init change delegate
		if (!IndividualObj->OnInitChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualInitChange))
		{
			IndividualObj->OnInitChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualInitChange);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on init delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		// Bind load change delegate
		if (!IndividualObj->OnLoadedChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualLoadedChange))
		{
			IndividualObj->OnLoadedChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualLoadedChange);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on loaded delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
		return true;
	}
	return false;
}


/* Private */
// Create the semantic individual
bool USLIndividualComponent::CreateIndividual()
{
	if (HasIndividual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	// Set semantic individual type depending on owner
	if (UClass* IndividualCls = FSLIndividualUtils::CreateIndividualObject(this, GetOwner(), IndividualObj))
	{	
		// Listen to updates to the individual
		BindDelegates();
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component has an unsuported type, self-destruction commenced.."),
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

