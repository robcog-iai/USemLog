// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLPerceivableIndividual.h"
#include "Materials/MaterialInstanceDynamic.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLPerceivableIndividual::USLPerceivableIndividual()
{
	VisualMaskMaterial = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), NULL, TEXT("Material'/USemLog/Individuals/M_VisualIndividualMask.M_VisualIndividualMask'"),
		NULL, LOAD_None, NULL));

	bIsMaskMaterialOn = false;
}

// Called before destroying the object.
void USLPerceivableIndividual::BeginDestroy()
{
	ApplyOriginalMaterials();
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLPerceivableIndividual::Init(bool bReset)
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
bool USLPerceivableIndividual::Load(bool bReset, bool bTryImport)
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

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}

// Save data to owners tag
bool USLPerceivableIndividual::ExportValues(bool bOverwrite)
{
	bool bMarkDirty = false;
	bMarkDirty = Super::ExportValues(bOverwrite) || bMarkDirty;
	if (!VisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, ImportTagType, "VisualMask", VisualMask, bOverwrite) || bMarkDirty;
	}
	if (!CalibratedVisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(ParentActor, ImportTagType, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLPerceivableIndividual::ImportValues(bool bOverwrite)
{
	bool bNewValue = false;
	if (Super::ImportValues(bOverwrite))
	{
		bNewValue = true;
	}

	if (ImportVisualMaskValue(bOverwrite))
	{
		bNewValue = true;
	}

	if (ImportCalibratedVisualMaskValue(bOverwrite))
	{
		bNewValue = true;
	}

	return bNewValue;
}

// Toggle between the visual mask and the origina materials
bool USLPerceivableIndividual::ToggleMaterials(bool bPrioritizeChildren /*= false*/)
{
	if (bIsMaskMaterialOn)
	{
		return ApplyOriginalMaterials();
	}
	else
	{
		return ApplyMaskMaterials(bPrioritizeChildren);
	}
}

// Set  visual mask
void USLPerceivableIndividual::SetVisualMaskValue(const FString& NewVisualMask, bool bApplyNewMaterial, bool bClearCalibratedVisualMask)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!VisualMask.Equals(NewVisualMask)) 
	{		
		VisualMask = NewVisualMask;
		OnNewVisualMaskValue.Broadcast(this, VisualMask);

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
			SetCalibratedVisualMaskValue("");
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

// Private init implementation
bool USLPerceivableIndividual::InitImpl()
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
bool USLPerceivableIndividual::LoadImpl(bool bTryImport)
{
	if (!IsVisualMaskValueSet())
	{
		if (bTryImport)
		{
			if (!ImportVisualMaskValue())
			{
				SetVisualMaskValue(GenerateNewVisualMask());
			}
		}
		else
		{
			SetVisualMaskValue(GenerateNewVisualMask());
		}
	}

	// Will be set to black if the visual mask is empty
	VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));

	return IsVisualMaskValueSet();
}

// Get class name, virtual since each invidiual type will have different name
FString USLPerceivableIndividual::GetClassName() const
{
	return GetTypeName();
}

// Randomly generates a new visual mask, does not guarantee uniqueness
FString USLPerceivableIndividual::GenerateNewVisualMask() const
{
	return FColor((uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f)).ToHex();
}

// Clear all values of the individual
void USLPerceivableIndividual::InitReset()
{
	LoadReset();
	ApplyOriginalMaterials();
	OriginalMaterials.Empty();
	SetIsInit(false);
	ClearDelegateBounds();
	Super::InitReset();
}

// Clear all data of the individual
void USLPerceivableIndividual::LoadReset()
{
	ClearVisualMaskValue();
	Super::LoadReset();
}

// Clear any bound delegates (called when init is reset)
void USLPerceivableIndividual::ClearDelegateBounds()
{
	OnNewVisualMaskValue.Clear();
}

// Import visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportVisualMaskValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsVisualMaskValueSet() || bOverwrite)
	{
		const FString PrevVal = VisualMask;
		SetVisualMaskValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "VisualMask"));
		bNewValue = !VisualMask.Equals(PrevVal);
		//if (!HasVisualMask())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d No VisualMask value could be imported from %s's tag.."),
		//		*FString(__FUNCTION__), __LINE__, *GetFullName());
		//}
	}
	return bNewValue;
}

// Import calibrated visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportCalibratedVisualMaskValue(bool bOverwrite)
{
	bool bNewValue = false;
	if (!IsCalibratedVisualMasValueSet() || bOverwrite)
	{
		const FString PrevVal = CalibratedVisualMask;
		SetCalibratedVisualMaskValue(FSLTagIO::GetValue(ParentActor, ImportTagType, "CalibratedVisualMask"));
		bNewValue = !CalibratedVisualMask.Equals(PrevVal);
		if (!IsCalibratedVisualMasValueSet())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No CalibratedVisualMask value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Apply color to the dynamic material
bool USLPerceivableIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial)
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
		return true;
	}
	return false;
}

// Check if the dynmic material is valid
bool USLPerceivableIndividual::HasValidDynamicMaterial() const
{
	return VisualMaskDynamicMaterial && VisualMaskDynamicMaterial->IsValidLowLevel() && !VisualMaskDynamicMaterial->IsPendingKill();
}

