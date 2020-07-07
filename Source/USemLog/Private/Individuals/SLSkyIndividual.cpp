// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLSkyIndividual.h"
#include "Atmosphere/AtmosphericFog.h"

// Ctor
USLSkyIndividual::USLSkyIndividual()
{
}

// Called before destroying the object.
void USLSkyIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLSkyIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLSkyIndividual::Init(bool bReset)
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
bool USLSkyIndividual::Load(bool bReset, bool bTryImport)
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



// Apply visual mask material
bool USLSkyIndividual::ApplyMaskMaterials(bool bPrioritizeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		ParentActor->SetIsTemporarilyHiddenInEditor(true);
		ParentActor->SetActorHiddenInGame(true);
		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLSkyIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		ParentActor->SetIsTemporarilyHiddenInEditor(false);
		ParentActor->SetActorHiddenInGame(false);
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Private init implementation
bool USLSkyIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLSkyIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Get class name, virtual since each invidiual type will have different name
FString USLSkyIndividual::GetClassName() const
{
	if (IsInit())
	{
		if (AAtmosphericFog* AAF = Cast<AAtmosphericFog>(ParentActor))
		{
			return "AtmosphericFog";
		}
		else if (ParentActor->GetName().Contains("SkySphere"))
		{
			return "SkySphere";
		}
	}
	return GetTypeName();
}

// Clear all values of the individual
void USLSkyIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLSkyIndividual::LoadReset()
{
	Super::LoadReset();
}
