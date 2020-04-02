// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLVisualIndividual.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"

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
}

// Create and set the dynamic material, the owners visual component
void USLVisualIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	if (VisualMaskMaterial)
	{
		VisualMaskDynamicMaterial = UMaterialInstanceDynamic::Create(VisualMaskMaterial, this);
		ApplyVisualMaskColorToDynamicMaterial();
	}
	if (GetSemOwner())
	{
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(GetSemOwner()))
		{
			if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
			{
				VisualStaticMeshComponent = SMC;
			}
		}
	}
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
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "VisualMask", VisualMask, bOverwrite);
	}

	if (!CalibratedVisualMask.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite);
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
		VisualMask = FSLTagIO::GetValue(SemOwner, TagTypeConst, "VisualMask");
	}

	if (CalibratedVisualMask.IsEmpty() || bOverwrite)
	{
		CalibratedVisualMask = FSLTagIO::GetValue(SemOwner, TagTypeConst, "CalibratedVisualMask");
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

