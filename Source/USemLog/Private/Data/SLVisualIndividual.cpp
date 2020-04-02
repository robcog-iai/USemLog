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
	bMaskMaterialOn = false;
}


// Called before destroying the object.
void USLVisualIndividual::BeginDestroy()
{
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLVisualIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Init individual
bool USLVisualIndividual::Init()
{	
	if (bIsInit)
	{
		return true;
	}

	if (!Super::Init())
	{
		return false;
	}

	if (!VisualMaskMaterial)
	{
		return false;
	}
	VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);
	ApplyVisualMaskColorToDynamicMaterial();

	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(GetSemanticOwner()))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			VisualSMC = SMC;
			OriginalMaterials = SMC->GetMaterials();
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

	bIsInit = true;
	return true;
}

// Save data to owners tag
bool USLVisualIndividual::SaveToTag(bool bOverwrite)
{
	if (!Super::SaveToTag(bOverwrite))
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
bool USLVisualIndividual::LoadFromTag(bool bOverwrite)
{
	if (!Super::LoadFromTag(bOverwrite))
	{
		return false;
	}

	if (VisualMask.IsEmpty() || bOverwrite)
	{
		VisualMask = FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "VisualMask");
	}

	if (CalibratedVisualMask.IsEmpty() || bOverwrite)
	{
		CalibratedVisualMask = FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "CalibratedVisualMask");
	}

	return true;
}

// All properties are set for runtime
bool USLVisualIndividual::IsRuntimeReady() const
{
	if (!Super::IsRuntimeReady())
	{
		return false;
	}

	return !VisualMask.IsEmpty() && !CalibratedVisualMask.IsEmpty();
}

// Apply visual mask material
bool USLVisualIndividual::ApplyVisualMaskMaterials()
{
	if (!bMaskMaterialOn && VisualSMC && VisualMaskDynamicMaterial)
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
	if (bMaskMaterialOn && VisualSMC && OriginalMaterials.Num())
	{
		if (VisualSMC->GetNumMaterials() == OriginalMaterials.Num())
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
void USLVisualIndividual::SetVisualMask(const FString& InVisualMask, bool bClearCalibratedValue)
{
	VisualMask = InVisualMask;

	// If there is a new visual mask, there calibrated value is invalid
	if (!VisualMask.Equals(InVisualMask) && bClearCalibratedValue) 
	{
		CalibratedVisualMask = ""; 
	}

	// Set the dynamic color value
	ApplyVisualMaskColorToDynamicMaterial();
}

// Apply color to the dynamic material
bool USLVisualIndividual::ApplyVisualMaskColorToDynamicMaterial()
{	
	if (VisualMaskDynamicMaterial && HasVisualMask())
	{
		VisualMaskDynamicMaterial->SetVectorParameterValue(FName("Color"), FColor::FromHex(VisualMask));
	}
	return false;
}

