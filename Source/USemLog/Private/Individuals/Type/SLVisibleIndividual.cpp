// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLVisibleIndividual.h"
#include "Materials/MaterialInstanceDynamic.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLVisibleIndividual::USLVisibleIndividual()
{
	VisualMaskMaterial = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), NULL, TEXT("Material'/USemLog/Individuals/M_VisualIndividualMask.M_VisualIndividualMask'"),
		NULL, LOAD_None, NULL));
	OriginalMaterials.Empty();
	bIsMaskMaterialOn = false;
}

// Called before destroying the object.
void USLVisibleIndividual::BeginDestroy()
{
	ApplyOriginalMaterials();
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLVisibleIndividual::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init(bReset) && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLVisibleIndividual::Load(bool bReset, bool bTryImport)
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

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

// Trigger values as new value broadcast
void USLVisibleIndividual::TriggerValuesBroadcast()
{
	Super::TriggerValuesBroadcast();

	OnNewValue.Broadcast(this, "VisualMask", VisualMask);
	//OnNewValue.Broadcast(this, "CalibratedVisualMask", CalibratedVisualMask);
}

// Save data to owners tag
bool USLVisibleIndividual::ExportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No valid parent actor found, could not export values (%s)"),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	bool bNewVal = false;
	bNewVal = Super::ExportValues(bOverwrite) || bNewVal;
	if (!VisualMask.IsEmpty())
	{
		bNewVal = FSLTagIO::AddKVPair(ParentActor, TagType, "VisualMask", VisualMask, bOverwrite) || bNewVal;
	}
	if (!CalibratedVisualMask.IsEmpty())
	{
		bNewVal = FSLTagIO::AddKVPair(ParentActor, TagType, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite) || bNewVal;
	}
	return bNewVal;
}

// Load data from owners tag
bool USLVisibleIndividual::ImportValues(bool bOverwrite)
{
	if (!HasValidParentActor())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No valid parent actor found, could not import values"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	bool bNewVal = false;
	if (Super::ImportValues(bOverwrite))
	{
		bNewVal = true;
	}

	if (ImportVisualMaskValue(bOverwrite))
	{
		bNewVal = true;
	}

	if (ImportCalibratedVisualMaskValue(bOverwrite))
	{
		bNewVal = true;
	}

	return bNewVal;
}

// Toggle between the visual mask and the origina materials
bool USLVisibleIndividual::ToggleMaterials(bool bIncludeChildren /*= false*/)
{
	if (bIsMaskMaterialOn)
	{
		return ApplyOriginalMaterials();
	}
	else
	{
		return ApplyMaskMaterials(bIncludeChildren);
	}
}

// Set  visual mask
void USLVisibleIndividual::SetVisualMaskValue(const FString& NewValue, bool bApplyNewMaterial, bool bClearCalibratedVisualMask)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!VisualMask.Equals(NewValue))
	{		
		VisualMask = NewValue;
		OnNewValue.Broadcast(this, "VisualMask", VisualMask);

		if (!IsVisualMaskValueSet() && IsLoaded())
		{
			SetIsLoaded(false);
		}
		else if (IsVisualMaskValueSet() && !IsLoaded())
		{
			// Check if the individual can now be loaded
			Load(false, false);
		}

		// The calibrated value will be obsolete for a new visual mask
		if (bClearCalibratedVisualMask)
		{
			ClearCalibratedVisualMaskValue();
		}

		// Update the dynamic material
		ApplyVisualMaskColorToDynamicMaterial();

		// If the mask visualization is active, dynamically update the colors
		if (bIsMaskMaterialOn && bApplyNewMaterial)
		{
			bIsMaskMaterialOn = false; // Force material reload
			ApplyMaskMaterials();
		}
	}
}

// Set the calibrated visual mask value
void USLVisibleIndividual::SetCalibratedVisualMaskValue(const FString& NewValue)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!CalibratedVisualMask.Equals(NewValue))
	{
		CalibratedVisualMask = NewValue;
		//OnNewValue.Broadcast(this, "CalibratedVisualMask", VisualMask);
	}
}

// Private init implementation
bool USLVisibleIndividual::InitImpl()
{
	if (!HasValidDynamicMaterial())
	{
		if (!VisualMaskMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask material asset, init failed.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
		VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);
	}

	if (!VisualMaskDynamicMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no VisualMaskDynamicMaterial, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	return true;
}

// Private load implementation
bool USLVisibleIndividual::LoadImpl(bool bTryImport)
{
	if (!IsVisualMaskValueSet())
	{
		if (bTryImport)
		{
			if (!ImportVisualMaskValue())
			{
				SetVisualMaskValue(GenerateNewRandomVisualMask());
			}
		}
		//else
		//{
		//	SetVisualMaskValue(GenerateNewRandomVisualMask());
		//}
	}

	// Will be set to black if the visual mask is empty
	VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));

	return IsVisualMaskValueSet();
}

// Clear all values of the individual
void USLVisibleIndividual::InitReset()
{
	LoadReset();
	ApplyOriginalMaterials();
	OriginalMaterials.Empty();
	SetIsInit(false);
}

// Clear all data of the individual
void USLVisibleIndividual::LoadReset()
{
	ClearVisualMaskValue();
	SetIsLoaded(false);
}


// Get class name, virtual since each invidiual type will have different name
FString USLVisibleIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Randomly generates a new visual mask, does not guarantee uniqueness
FString USLVisibleIndividual::GenerateNewRandomVisualMask() const
{
	return FColor((uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f)).ToHex();
}

// Import visual mask from tag, true if new value is written
bool USLVisibleIndividual::ImportVisualMaskValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsVisualMaskValueSet() || bOverwrite)
	{
		const FString PrevVal = VisualMask;
		SetVisualMaskValue(FSLTagIO::GetValue(ParentActor, TagType, "VisualMask"));
		bNewValue = !VisualMask.Equals(PrevVal);
	}
	return bNewValue;
}

// Import calibrated visual mask from tag, true if new value is written
bool USLVisibleIndividual::ImportCalibratedVisualMaskValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsCalibratedVisualMaskValueSet() || bOverwrite)
	{
		const FString PrevVal = CalibratedVisualMask;
		SetCalibratedVisualMaskValue(FSLTagIO::GetValue(ParentActor, TagType, "CalibratedVisualMask"));
		bNewValue = !CalibratedVisualMask.Equals(PrevVal);
	}
	return bNewValue;
}

// Apply color to the dynamic material
bool USLVisibleIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial)
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
		return true;
	}
	return false;
}

// Check if the dynmic material is valid
bool USLVisibleIndividual::HasValidDynamicMaterial() const
{
	return VisualMaskDynamicMaterial && VisualMaskDynamicMaterial->IsValidLowLevel() && !VisualMaskDynamicMaterial->IsPendingKill();
}

