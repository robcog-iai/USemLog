// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLPerceivableIndividual.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLPerceivableIndividual::USLPerceivableIndividual()
{
	VisualMaskMaterial = Cast<UMaterial>(StaticLoadObject(
		UMaterial::StaticClass(), NULL, TEXT("Material'/USemLog/Individual/M_VisualIndividualMask.M_VisualIndividualMask'"),
		NULL, LOAD_None, NULL));

	SemanticOwner = nullptr;
	bIsInit = false;
	bIsLoaded = false;
	bMaskMaterialOn = false;
}

// Called before destroying the object.
void USLPerceivableIndividual::BeginDestroy()
{
	ApplyOriginalMaterials();
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLPerceivableIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLPerceivableIndividual::Init(bool bReset)
{
	if (bReset)
	{
		SetIsInit(false, false);
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init(bReset) && InitImpl());
	return IsInit();
}


// Load semantic data
bool USLPerceivableIndividual::Load(bool bReset)
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load component individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset) && LoadImpl());
	return IsLoaded();
}

// Save data to owners tag
bool USLPerceivableIndividual::ExportToTag(bool bOverwrite)
{
	bool bMarkDirty = false;
	bMarkDirty = Super::ExportToTag(bOverwrite) || bMarkDirty;
	if (!VisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "VisualMask", VisualMask, bOverwrite) || bMarkDirty;
	}
	if (!CalibratedVisualMask.IsEmpty())
	{
		bMarkDirty = FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite) || bMarkDirty;
	}
	return bMarkDirty;
}

// Load data from owners tag
bool USLPerceivableIndividual::ImportFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (Super::ImportFromTag(bOverwrite))
	{
		bNewValue = true;
	}

	if (ImportVisualMaskFromTag(bOverwrite))
	{
		bNewValue = true;
	}

	if (ImportCalibratedVisualMaskFromTag(bOverwrite))
	{
		bNewValue = true;
	}

	return bNewValue;
}

// Apply visual mask material
bool USLPerceivableIndividual::ApplyVisualMaskMaterials(bool bReload)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bMaskMaterialOn || bReload)
	{
		for (int32 MatIdx = 0; MatIdx < VisualSMC->GetNumMaterials(); ++MatIdx)
		{
			VisualSMC->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
		}
		bMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLPerceivableIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bMaskMaterialOn)
	{
		int32 MatIdx = 0;
		for (const auto& Mat : OriginalMaterials)
		{
			VisualSMC->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}
		bMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Toggle between the visual mask and the origina materials
bool USLPerceivableIndividual::ToggleMaterials()
{
	if (bMaskMaterialOn)
	{
		return ApplyOriginalMaterials();
	}
	else
	{
		return ApplyVisualMaskMaterials();
	}
}

// Set  visual mask
void USLPerceivableIndividual::SetVisualMask(const FString& NewVisualMask, bool bApplyNewMaterial, bool bClearCalibratedVisualMask)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!VisualMask.Equals(NewVisualMask)) 
	{		
		VisualMask = NewVisualMask;
		OnNewVisualMaskValue.Broadcast(this, VisualMask);
		SetIsLoaded(LoadImpl(false));

		// The calibrated value will be obsolete for a new visual mask
		if (bClearCalibratedVisualMask)
		{
			SetCalibratedVisualMask("");
		}

		// Update the dynamic material
		ApplyVisualMaskColorToDynamicMaterial();

		// If the mask visualization is active, dynamically update the colors
		if (bMaskMaterialOn && bApplyNewMaterial)
		{
			ApplyVisualMaskMaterials(true);
		}
	}
}

// Apply color to the dynamic material
bool USLPerceivableIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial && HasVisualMask())
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
		return true;
	}
	return false;
}

// Import visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportVisualMaskFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (!HasVisualMask() || bOverwrite)
	{
		const FString PrevVal = VisualMask;
		SetVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "VisualMask"));
		bNewValue = !VisualMask.Equals(PrevVal);
		if (!HasVisualMask())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No VisualMask value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Import calibrated visual mask from tag, true if new value is written
bool USLPerceivableIndividual::ImportCalibratedVisualMaskFromTag(bool bOverwrite)
{
	bool bNewValue = false;
	if (!HasCalibratedVisualMask() || bOverwrite)
	{
		const FString PrevVal = CalibratedVisualMask;
		SetCalibratedVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "CalibratedVisualMask"));
		bNewValue = !CalibratedVisualMask.Equals(PrevVal);
		if (!HasCalibratedVisualMask())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No CalibratedVisualMask value could be imported from %s's tag.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return bNewValue;
}

// Private init implementation
bool USLPerceivableIndividual::InitImpl()
{
	if (!VisualMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask material asset, init failed.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
	VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);

	if (!VisualMaskDynamicMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no VisualMaskDynamicMaterial, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(SemanticOwner))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			VisualSMC = SMC;
			OriginalMaterials = SMC->GetMaterials();
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no StaticMeshComponent, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s SemanticOwner is not a StaticMeshActor, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
}

// Private load implementation
bool USLPerceivableIndividual::LoadImpl(bool bTryImportFromTags)
{
	bool bSuccess = true;

	if (!HasVisualMask())
	{
		if (bTryImportFromTags)
		{
			if (!ImportVisualMaskFromTag())
			{
				bSuccess = false;
			}
		}
		else
		{
			bSuccess = false;
		}
	}

	// Will be set to black if the visual mask is empty
	VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));

	return bSuccess;
}
