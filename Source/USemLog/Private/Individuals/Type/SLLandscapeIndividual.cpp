// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLLandscapeIndividual.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape.h"
#include "LandscapeProxy.h"

// Ctor
USLLandscapeIndividual::USLLandscapeIndividual()
{
	LandscapeActor = nullptr;
}

// Called before destroying the object.
void USLLandscapeIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLLandscapeIndividual::Init(bool bReset)
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
bool USLLandscapeIndividual::Load(bool bReset, bool bTryImport)
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

// Apply visual mask material
bool USLLandscapeIndividual::ApplyMaskMaterials(bool bIncludeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
#if WITH_EDITOR		
		ParentActor->SetIsTemporarilyHiddenInEditor(true);
#endif // WITH_EDITOR
		ParentActor->SetActorHiddenInGame(true);
		
//		//LandscapeActor->LandscapeMaterial = VisualMaskDynamicMaterial;
//#if WITH_EDITOR
//		//LandscapeActor->EditorSetLandscapeMaterial(VisualMaskDynamicMaterial);
//
//		UProperty* MaterialProperty = FindField<UProperty>(ALandscape::StaticClass(), "LandscapeMaterial");
//		LandscapeActor->PreEditChange(MaterialProperty);
//		LandscapeActor->LandscapeMaterial = VisualMaskDynamicMaterial;
//		FPropertyChangedEvent PropertyChangedEvent(MaterialProperty);
//		LandscapeActor->PostEditChangeProperty(PropertyChangedEvent);
//#endif // WITH_EDITOR

		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLLandscapeIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
#if WITH_EDITOR		
		ParentActor->SetIsTemporarilyHiddenInEditor(false);
#endif // WITH_EDITOR
		ParentActor->SetActorHiddenInGame(false);

//		if (OriginalMaterials.Num() > 0)
//		{
//#if WITH_EDITOR
//			//LandscapeActor->EditorSetLandscapeMaterial(OriginalMaterials[0]);
//
//			UProperty* MaterialProperty = FindField<UProperty>(ALandscape::StaticClass(), "LandscapeMaterial");
//			LandscapeActor->PreEditChange(MaterialProperty);
//			LandscapeActor->LandscapeMaterial = OriginalMaterials[0];
//			FPropertyChangedEvent PropertyChangedEvent(MaterialProperty);
//			LandscapeActor->PostEditChangeProperty(PropertyChangedEvent);
//#endif // WITH_EDITOR
//		}

		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLLandscapeIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLLandscapeIndividual::InitImpl()
{
	if (HasValidLandscapeActor() || SetLandscapeActor())
	{

		OriginalMaterials.Empty();
		OriginalMaterials.Add(LandscapeActor->LandscapeMaterial);
		return true;
	}
	return false;
}

// Private load implementation
bool USLLandscapeIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLLandscapeIndividual::InitReset()
{
	LoadReset();
	LandscapeActor = nullptr;
	OriginalMaterials.Empty();
	SetIsInit(false);
}

// Clear all data of the individual
void USLLandscapeIndividual::LoadReset()
{
	SetIsLoaded(false);
}


// Check if the static mesh component is set
bool USLLandscapeIndividual::HasValidLandscapeActor() const
{
	return LandscapeActor && LandscapeActor->IsValidLowLevel() && !LandscapeActor->IsPendingKill();
}

// Set the static mesh component
bool USLLandscapeIndividual::SetLandscapeActor()
{
	if (ALandscape* AsLandscape = Cast<ALandscape>(ParentActor))
	{
		LandscapeActor = AsLandscape;	
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's parent actor is not a ALandscape, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
}
