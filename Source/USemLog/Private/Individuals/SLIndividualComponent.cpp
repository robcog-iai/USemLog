// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLBoneIndividual.h"
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

	bUpdateIndivdualsCachedPoseButtonHack = false;
}

// Do any object-specific cleanup required immediately after loading an object.
void USLIndividualComponent::PostLoad()
{
	Super::PostLoad();
	if (!IsConnected())
	{
		Connect();
	}
}

// Called when a component is created(not loaded).This can happen in the editor or during gameplay
void USLIndividualComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	// Check if actor already has a semantic data component
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(USLIndividualComponent::StaticClass(), Components);
	for (const auto AC : Components)
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
#else
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
#endif
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLIndividualComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bUpdateIndivdualsCachedPoseButtonHack))
	{
		bUpdateIndivdualsCachedPoseButtonHack = false;
		UpdateCachedPoses();
	}

}
#endif // WITH_EDITOR

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
bool USLIndividualComponent::ToggleVisualMaskVisibility(bool bIncludeChildren)
{
	if (HasValidIndividual())
	{
		if (USLVisibleIndividual* VI = Cast<USLVisibleIndividual>(IndividualObj))
		{
			return VI->ToggleMaterials(bIncludeChildren);
		}
	}

	return false;
}

// Re-broadcast all available values
bool USLIndividualComponent::TriggerIndividualValuesBroadcast()
{
	if (HasValidIndividual())
	{
		IndividualObj->TriggerValuesBroadcast();
		OnChildrenNumChanged.Broadcast(this, IndividualChildren.Num(), AttachableIndividualChildren.Num());
		return true;
	}
	return false;
}

// Update individuals and their children cached poses
bool USLIndividualComponent::UpdateCachedPoses()
{
	if (!HasValidIndividual())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Component %s has no valid individual"), *FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	IndividualObj->UpdateCachedPose();

	for (const auto& Child : IndividualChildren)
	{
		Child->UpdateCachedPose();
	}
	return true;
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

			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(IndividualObj))
			{
				for (const auto& Child : GetIndividualChildren())
				{
					Child->GenerateNewIdValue();
				}
			}

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

			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(IndividualObj))
			{
				for (const auto& Child : GetIndividualChildren())
				{
					Child->ClearIdValue();
				}
			}
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
			
			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(IndividualObj))
			{
				for (const auto& Child : GetIndividualChildren())
				{
					// TODO Class should not be changed
					//Child->SetDefaultClassValue();
				}
			}

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

			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(IndividualObj))
			{
				for (const auto& Child : GetIndividualChildren())
				{
					// TODO Class should not be changed
					//Child->ClearClassValue();
				}
			}

			return true;
		}
	}
	return false;
}

// Write visual mask value (if perceivable)
bool USLIndividualComponent::WriteVisualMask(const FString& Value, bool bOverwrite, const TArray<FString>& ChildrenValues)
{
	if (HasValidIndividual())
	{
		if (USLVisibleIndividual* VI = Cast<USLVisibleIndividual>(IndividualObj))
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
		if (USLVisibleIndividual* VI = Cast<USLVisibleIndividual>(IndividualObj))
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
	SetIsConnected(false);
	LoadReset();
	SetIsInit(false);
	if (HasValidIndividual())
	{
		IndividualObj->Init(true);
	}
	ClearIndividualChildren();
	ClearDelegates();
}

// Clear all data of the individual
void USLIndividualComponent::LoadReset()
{
	SetIsLoaded(false);
	if (HasValidIndividual())
	{
		IndividualObj->Load(true, false);
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
	OnChildValueChanged.Clear();
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

// Set any individual children if available
void USLIndividualComponent::SetIndividualChildren()
{
	if (!HasValidIndividual())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no valid individual, cannot load any children.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}

	if (HasIndividualChildren())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has children, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		ClearIndividualChildren();
	}

	if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(IndividualObj))
	{
		IndividualChildren = SkI->GetChildrenIndividuals();

		// Check is child is attachable
		for (const auto& Child : IndividualChildren)
		{
			FName AttachmentLocationName = NAME_None;
			if (SkI->IsChildAttachable(Child, AttachmentLocationName))
			{
				if (AttachmentLocationName != NAME_None)
				{
					AttachableIndividualChildren.Add(Child, AttachmentLocationName);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s's child individual %s is attachable with the location name NONE, this should not happen.."),
						*FString(__FUNCTION__), __LINE__, *GetFullName(), *Child->GetFullName());
				}
			}
		}
	}
	// TODO robot data
	//else if()

	BindChildrenDelegates();
	OnChildrenNumChanged.Broadcast(this, IndividualChildren.Num(), AttachableIndividualChildren.Num());
}

// Clear any cached individual children
void USLIndividualComponent::ClearIndividualChildren()
{
	UnBindChildrenDelegates();
	IndividualChildren.Empty();
	AttachableIndividualChildren.Empty();
	OnChildrenNumChanged.Broadcast(this, IndividualChildren.Num(), AttachableIndividualChildren.Num());
}

// Create individual if not created and forward init call
bool USLIndividualComponent::InitImpl()
{
	if (HasValidIndividual() || CreateIndividual())
	{
		Connect();
		if(IndividualObj->Init(false))
		{
			// Children are only available after init
			SetIndividualChildren();
			return true;
		}
	}
	else
	{
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not create an individual, this should not happen.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Forward the laod call to the individual object
bool USLIndividualComponent::LoadImpl(bool bTryImport)
{
	if (HasValidIndividual())
	{
		return IndividualObj->Load(false, bTryImport);
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
	if (!IndividualObj->OnNewValue.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualNewValue))
	{
		IndividualObj->OnNewValue.AddDynamic(this, &USLIndividualComponent::OnIndividualNewValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component new value delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return true;
}

// Sync states with the individuals children
bool USLIndividualComponent::BindChildrenDelegates()
{
	// TODO bind for all the information (init, load, connect)
	for (const auto& IndividualChild : IndividualChildren)
	{
		if (!IndividualChild->OnNewValue.IsAlreadyBound(this, &USLIndividualComponent::OnChildIndividualNewValue))
		{
			IndividualChild->OnNewValue.AddDynamic(this, &USLIndividualComponent::OnChildIndividualNewValue);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's child %s value delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *IndividualChild->GetFullName());
		}
	}
	return true;
}

// Clear children delegates
bool USLIndividualComponent::UnBindChildrenDelegates()
{
	for (const auto& IndividualChild : IndividualChildren)
	{
		if (IndividualChild->OnNewValue.IsAlreadyBound(this, &USLIndividualComponent::OnChildIndividualNewValue))
		{
			IndividualChild->OnNewValue.RemoveDynamic(this, &USLIndividualComponent::OnChildIndividualNewValue);
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
	if (bNewValue)
	{
		Init();
	}
	else
	{
		SetIsInit(false);
	}
}

// Triggered when the semantic individual loaded flag changes
void USLIndividualComponent::OnIndividualLoadedChange(USLBaseIndividual* Individual, bool bNewValue)
{
	if (bNewValue)
	{
		Load();
	}
	else
	{
		SetIsLoaded(false);
	}
}

// Triggered when an individual value is changed
void USLIndividualComponent::OnIndividualNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue)
{
	// Forward the data to any listeners
	OnValueChanged.Broadcast(this, Key, NewValue);
}

// Triggered when a child individual value is changed
void USLIndividualComponent::OnChildIndividualNewValue(USLBaseIndividual* Individual, const FString& Key, const FString& NewValue)
{
	OnChildValueChanged.Broadcast(this, Individual, Key, NewValue);
}

// Triggered with the delegates are cleared on the individual object (required reconnection afterwards)
void USLIndividualComponent::OnIndividualDelegatesCleared(USLBaseIndividual* Individual)
{
	SetIsConnected(false);
}

