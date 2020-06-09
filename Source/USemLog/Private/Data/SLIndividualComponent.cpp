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

	SemanticIndividual = nullptr;

	bIsInit = false;
	bIsLoaded = false;

	bOverwriteEditChanges = false;
	bExportToTagButton = false;
	bImportFromTagButton = false;
	bToggleVisualMaskMaterial = false;
}

// Called before destroying the object.
void USLIndividualComponent::BeginDestroy()
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		SemanticIndividual->ConditionalBeginDestroy();
	}

	OnDestroyed.Broadcast(this);

	Super::BeginDestroy();
}

// Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
void USLIndividualComponent::PostInitProperties()
{
	Super::PostInitProperties();

	//// Check if actor already has a semantic data component
	//for (const auto AC : GetOwner()->GetComponentsByClass(USLIndividualComponent::StaticClass()))
	//{
	//	if (AC != this)
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("%s::%d %s already has a semantic data component (%s), self-destruction commenced.."),
	//			*FString(__FUNCTION__), __LINE__, *GetOwner()->GetName(), *AC->GetName());
	//		//DestroyComponent();
	//		ConditionalBeginDestroy();
	//		return;
	//	}
	//}

	//Init();
	//Load();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLIndividualComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Convert datatype
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, IndividualType))
	{
		FSLIndividualUtils::ConvertIndividualObject(SemanticIndividual, IndividualType);
	}

	/* Button workaround triggers */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bExportToTagButton))
	{
		bExportToTagButton = false;
		ExportToTag(bOverwriteEditChanges);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bImportFromTagButton))
	{
		bImportFromTagButton = false;
		ImportFromTag(bOverwriteEditChanges);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLIndividualComponent, bToggleVisualMaskMaterial))
	{
		bToggleVisualMaskMaterial = false;
		ToggleVisualMaskVisibility();
	}
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

	Init();
	Load();
}

// Called when the game starts
void USLIndividualComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Set owner and individual
bool USLIndividualComponent::Init(bool bReset)
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		bIsInit = SemanticIndividual->Init(bReset);
		return bIsInit;
	}
	else
	{
		if (CreateIndividual())
		{
			bIsInit = SemanticIndividual->Init(bReset);
			return bIsInit;
		}
	}
	return false;
}

// Load individual
bool USLIndividualComponent::Load(bool bReset)
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		bIsLoaded = SemanticIndividual->Load(bReset);
		return bIsLoaded;
	}
	else
	{
		if (CreateIndividual())
		{
			bIsLoaded = SemanticIndividual->Load(bReset);
			return bIsLoaded;
		}
	}
	return false;
}


/* Functionalities */
// Save data to owners tag
bool USLIndividualComponent::ExportToTag(bool bOverwrite)
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		return SemanticIndividual->ExportToTag(bOverwrite);
	}
	return false;
}

// Load data from owners tag
bool USLIndividualComponent::ImportFromTag(bool bOverwrite)
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		return SemanticIndividual->ImportFromTag(bOverwrite);
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

/* Private */
// Create the semantic individual
bool USLIndividualComponent::CreateIndividual()
{
	if (SemanticIndividual && SemanticIndividual->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic individual already exists, this should not happen.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}

	// Set semantic individual type depending on owner
	if (UClass* IndividualCls = FSLIndividualUtils::CreateIndividualObject(this, GetOwner(), SemanticIndividual))
	{		
		// Cache the current individual class type
		IndividualType = IndividualCls;
		SemanticIndividual->OnInitChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualInitChange);
		SemanticIndividual->OnLoadedChanged.AddDynamic(this, &USLIndividualComponent::OnIndividualLoadedChange);
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
	bIsInit = bNewValue;
}

// Triggered when the semantic individual loaded flag changes
void USLIndividualComponent::OnIndividualLoadedChange(USLBaseIndividual* Individual, bool bNewValue)
{
	bIsLoaded = bNewValue;
}

