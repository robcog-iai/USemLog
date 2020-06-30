// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLConstraintIndividual.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLConstraintIndividual::USLConstraintIndividual()
{
	ParentActor = nullptr;
	bIsInit = false;
	bIsLoaded = false;
}

// Called before destroying the object.
void USLConstraintIndividual::BeginDestroy()
{
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLConstraintIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLConstraintIndividual::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init() && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLConstraintIndividual::Load(bool bReset, bool bTryImportFromTags)
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load component individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load() && LoadImpl(bTryImportFromTags));
	return IsLoaded();
}

// Save data to owners tag
bool USLConstraintIndividual::ExportToTag(bool bOverwrite)
{
	bool bMarkDirty = false;
	bMarkDirty = Super::ExportToTag(bOverwrite) || bMarkDirty;
	return bMarkDirty;
}

// Load data from owners tag
bool USLConstraintIndividual::ImportFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (Super::ImportFromTag(bOverwrite))
	{
		bNewValue = true;
	}
	return bNewValue;
}


// Private init implementation
bool USLConstraintIndividual::InitImpl()
{
	if (HasValidParentActor())
	{
		if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(ParentActor))
		{
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s parent actor is not a physics constraint actor, init failed.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no valid parent actor, init failed.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
	return false;
}

// Private load implementation
bool USLConstraintIndividual::LoadImpl(bool bTryImportFromTags)
{
	// bool bRetValue = true;

	// if (!HasVisualMask())
	// {
		// if (bTryImportFromTags)
		// {
			// if (!ImportVisualMaskFromTag())
			// {
				// bRetValue = false;
			// }
		// }
		// else
		// {
			// bRetValue = false;
		// }
	// }

	// // Will be set to black if the visual mask is empty
	// VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));

	// return bRetValue;
	
	return false;
}

// Clear all values of the individual
void USLConstraintIndividual::InitReset()
{
	// LoadReset();
	// ApplyOriginalMaterials();
	// VisualSMC = nullptr;
	// OriginalMaterials.Empty();
	// SetIsInit(false);
	// ClearDelegateBounds();
	Super::InitReset();
}

// Clear all data of the individual
void USLConstraintIndividual::LoadReset()
{
	// SetVisualMask("");
	Super::LoadReset();
}

// Clear any bound delegates (called when init is reset)
void USLConstraintIndividual::ClearDelegateBounds()
{
}

