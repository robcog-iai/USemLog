// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLVisualIndividual.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Ctor
USLVisualIndividual::USLVisualIndividual()
{
	// Load the mask material
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialAsset(
		TEXT("Material'/USemLog/Individual/M_VisualIndividualMask.M_VisualIndividualMask'"));
	if (MaterialAsset.Succeeded())
	{
		VisualMaskMaterial = MaterialAsset.Object;		
		////VisualMaskDynamicMaterial = CreateDefaultSubobject<UMaterialInstanceDynamic>(TEXT("VisualMaskDynamicMaterial"));
		////VisualMaskDynamicMaterial->SetParentInternal(MaterialAsset.Object, false);
	}

	bIsInit = false;
	bIsLoaded = false;

	bMaskMaterialOn = false;
}

// Called before destroying the object.
void USLVisualIndividual::BeginDestroy()
{
	ApplyOriginalMaterials();
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLVisualIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLVisualIndividual::Init(bool bForced)
{
	if (bForced)
	{
		bIsInit = false;
	}

	if (IsInit())
	{
		return true;
	}

	if (!Super::Init(bForced))
	{
		return false;
	}

	bIsInit = InitImpl();
	return bIsInit;
}

// Check if individual is initialized
bool USLVisualIndividual::IsInit() const
{
	return bIsInit && Super::IsInit();
}

// Load semantic data
bool USLVisualIndividual::Load(bool bForced)
{
	if (bForced)
	{
		bIsLoaded = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		return false;
	}

	if (!Super::Load(bForced))
	{
		if (!Init(bForced))
		{
			return false;
		}
	}

	bIsLoaded = LoadImpl();
	return bIsLoaded;
}

// Check if semantic data is succesfully loaded
bool USLVisualIndividual::IsLoaded() const
{
	return bIsLoaded /*&& Super::IsLoaded()*/;
}

// Save data to owners tag
bool USLVisualIndividual::ExportToTag(bool bOverwrite)
{
	if (!Super::ExportToTag(bOverwrite))
	{
		return false;
	}

	if (!VisualMask.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "VisualMask", VisualMask, bOverwrite);
	}

	if (!CalibratedVisualMask.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite);
	}

	return true;
}

// Load data from owners tag
bool USLVisualIndividual::ImportFromTag(bool bOverwrite)
{
	if (!Super::ImportFromTag(bOverwrite))
	{
		return false;
	}

	if (VisualMask.IsEmpty() || bOverwrite)
	{
		SetVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "VisualMask"));		
	}

	if (CalibratedVisualMask.IsEmpty() || bOverwrite)
	{
		SetCalibratedVisualMask(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "CalibratedVisualMask"));
	}

	return true;
}

// Apply visual mask material
bool USLVisualIndividual::ApplyVisualMaskMaterials(bool bReload)
{

	if (!bIsLoaded)
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
bool USLVisualIndividual::ApplyOriginalMaterials()
{
	if (!bIsLoaded)
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
bool USLVisualIndividual::ToggleMaterials()
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
void USLVisualIndividual::SetVisualMask(const FString& InVisualMask, bool bReload, bool bClearCalibratedValue)
{
	// Clear the calibrated color in case of a new visual mask value
	if (!VisualMask.Equals(InVisualMask) && bClearCalibratedValue) 
	{
		CalibratedVisualMask = ""; 
	}
	
	// Set the new visual mask
	VisualMask = InVisualMask;

	// Update the dynamic material
	ApplyVisualMaskColorToDynamicMaterial();
	
	// If the mask visualization is active, dynamically update the colors
	if (bMaskMaterialOn && bReload)
	{
		ApplyVisualMaskMaterials(true);
	}
}

// Apply color to the dynamic material
bool USLVisualIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial && HasVisualMask())
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
		return true;
	}
	return false;
}

// Private init implementation
bool USLVisualIndividual::InitImpl()
{
	if (!VisualMaskMaterial)
	{
		return false;
	}
	VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);

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
			return false;
		}
	}
	else
	{
		return false;
	}
}

// Private load implementation
bool USLVisualIndividual::LoadImpl()
{
	if (!HasVisualMask())
	{
		return false;
	}
	VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
	return true;
}
