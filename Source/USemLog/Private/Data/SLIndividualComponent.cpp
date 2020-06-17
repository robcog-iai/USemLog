// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualComponent.h"
#include "Data/SLSkeletalIndividual.h"
#include "Data/SLPerceivableIndividual.h"
#include "Data/SLIndividualUtils.h"

// Sets default values for this component's properties
USLIndividualComponent::USLIndividualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ChildIndividual = nullptr;

	bIsInit = false;
	bIsLoaded = false;

	//bOverwriteEditChanges = false;
	//bExportToTagButton = false;
	//bImportFromTagButton = false;
	//bToggleVisualMaskMaterial = false;
}

// Called before destroying the object.
void USLIndividualComponent::BeginDestroy()
{
	SetIsInit(false);
	SetIsLoaded(false);

	OnDestroyed.Broadcast(this);

	if (HasIndividual())
	{
		ChildIndividual->ConditionalBeginDestroy();
	}

	Super::BeginDestroy();
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLIndividualComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//// Get the changed property name
	//FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
	//	PropertyChangedEvent.Property->GetFName() : NAME_None;

	//// Convert datatype
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, IndividualType))
	//{
	//	FSLIndividualUtils::ConvertIndividualObject(Individual, IndividualType);
	//}

	///* Button workaround triggers */
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bExportToTagButton))
	//{
	//	bExportToTagButton = false;
	//	ExportToTag(bOverwriteEditChanges);
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bImportFromTagButton))
	//{
	//	bImportFromTagButton = false;
	//	ImportFromTag(bOverwriteEditChanges);
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bToggleVisualMaskMaterial))
	//{
	//	bToggleVisualMaskMaterial = false;
	//	ToggleVisualMaskVisibility();
	//}
}
#endif // WITH_EDITOR

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

// Called when the game starts
void USLIndividualComponent::BeginPlay()
{
	Super::BeginPlay();
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

	SetIsInit(InitImpl());
	return IsInit();
}

// Load individual
bool USLIndividualComponent::Load(bool bReset)
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

	SetIsLoaded(LoadImpl());
	return IsLoaded();
}


/* Functionalities */
// Save data to owners tag
bool USLIndividualComponent::ExportToTag(bool bOverwrite)
{
	if (ChildIndividual && ChildIndividual->IsValidLowLevel())
	{
		return ChildIndividual->ExportToTag(bOverwrite);
	}
	return false;
}

// Load data from owners tag
bool USLIndividualComponent::ImportFromTag(bool bOverwrite)
{
	if (ChildIndividual && ChildIndividual->IsValidLowLevel())
	{
		return ChildIndividual->ImportFromTag(bOverwrite);
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
		return ChildIndividual->Init(bReset);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s Could not create individual.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
	return false;
}

bool USLIndividualComponent::LoadImpl(bool bReset)
{
	if (HasIndividual())
	{
		return ChildIndividual->Load(bReset);
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d %s This should not happen, and idividual should be created here.."),
		*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
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
	if (UClass* IndividualCls = FSLIndividualUtils::CreateIndividualObject(this, GetOwner(), ChildIndividual))
	{		
		// Cache the current individual class type
		IndividualType = IndividualCls;
		
		// Bind init change delegate
		if (!ChildIndividual->OnInitChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualInitChange))
		{
			ChildIndividual->OnInitChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualInitChange);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on init delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

		// Bind load change delegate
		if (!ChildIndividual->OnLoadedChanged.IsAlreadyBound(this, &USLIndividualComponent::OnIndividualLoadedChange))
		{
			ChildIndividual->OnLoadedChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualLoadedChange);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual component on loaded delegate is already bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
		}

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

