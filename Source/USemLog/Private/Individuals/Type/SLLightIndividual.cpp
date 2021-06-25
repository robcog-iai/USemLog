// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLLightIndividual.h"

// Ctor
USLLightIndividual::USLLightIndividual()
{
}

// Called before destroying the object.
void USLLightIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLLightIndividual::Init(bool bReset)
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
bool USLLightIndividual::Load(bool bReset, bool bTryImport)
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

// Get class name, virtual since each invidiual type will have different name
FString USLLightIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLLightIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLLightIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLLightIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLLightIndividual::LoadReset()
{
	SetIsLoaded(false);
}
