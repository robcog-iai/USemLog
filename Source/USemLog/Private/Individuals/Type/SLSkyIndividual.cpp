// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLSkyIndividual.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/SkyAtmosphereComponent.h"

// Ctor
USLSkyIndividual::USLSkyIndividual()
{
	bIsMovable = false;
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
	//Init();
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

	SetIsInit(Super::Init(bReset) && InitImpl());
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
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}



// Apply visual mask material
bool USLSkyIndividual::ApplyMaskMaterials(bool bIncludeChildren /*= false*/)
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
#if WITH_EDITOR		
		ParentActor->SetIsTemporarilyHiddenInEditor(false);
#endif // WITH_EDITOR
		ParentActor->SetActorHiddenInGame(false);
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLSkyIndividual::CalcDefaultClassValue()
{
	if (IsInit())
	{
		//if (AAtmosphericFog* AAF = Cast<AAtmosphericFog>(ParentActor))
		//{
		//	return "AtmosphericFog";
		//}
		if (ASkyAtmosphere* AAF = Cast<ASkyAtmosphere>(ParentActor))
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

// Clear all values of the individual
void USLSkyIndividual::InitReset()
{
	LoadReset();
	SetIsInit(false);
}

// Clear all data of the individual
void USLSkyIndividual::LoadReset()
{
	SetIsLoaded(false);
}
