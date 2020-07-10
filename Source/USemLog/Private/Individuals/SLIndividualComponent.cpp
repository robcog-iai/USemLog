// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualComponent.h"
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
	IndividualObj = nullptr;

	bDelegatesBound = false;
}

// Called after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called
void USLIndividualComponent::OnRegister()
{
	Super::OnRegister();

	// Re-bind delegates if the init state 
	if (IsInit() && !bDelegatesBound)
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

	if (!HasValidIndividual())
	{
		CreateIndividual();
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
bool USLIndividualComponent::Load(bool bReset, bool bTryImport)
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

	SetIsLoaded(LoadImpl(bReset, bTryImport));
	return IsLoaded();
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
	if (HasValidIndividual() || CreateIndividual())
	{
		return IndividualObj->Init(bReset);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s Could not create individual.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	return false;
}

// Forward the laod call to the individual object
bool USLIndividualComponent::LoadImpl(bool bReset, bool bTryImport)
{
	if (HasValidIndividual())
	{
		return IndividualObj->Load(bReset, bTryImport);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s This should not happen, and idividual should be created here.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	return false;
}

// Sync states with the individual
bool USLIndividualComponent::BindDelegates()
{
	if (HasValidIndividual())
	{
		// Bind init change delegate
		if (!IndividualObj->OnInitChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualInitChange))
		{
			IndividualObj->OnInitChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualInitChange);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on init delegate is already bound, this should not happen.."),
			//	*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		// Bind load change delegate
		if (!IndividualObj->OnLoadedChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualLoadedChange))
		{
			IndividualObj->OnLoadedChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualLoadedChange);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on loaded delegate is already bound, this should not happen.."),
			//	*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}
		bDelegatesBound = true;
		return true;
	}
	bDelegatesBound = false;
	return false;
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

		// Listen to updates to the individual
		BindDelegates();
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

