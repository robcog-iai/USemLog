// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLLightIndividual.h"

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

	SetIsInit(Super::Init() && InitImpl());
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}

// Get class name, virtual since each invidiual type will have different name
FString USLLightIndividual::CalcDefaultClassValue() const
{
	return GetTypeName();
}

// Clear all values of the individual
void USLLightIndividual::InitReset()
{
	SetIsInit(false);
	ClearDelegates();
	Super::InitReset();
}

// Clear all data of the individual
void USLLightIndividual::LoadReset()
{
	Super::LoadReset();
}

// Clear any bound delegates (called when init is reset)
void USLLightIndividual::ClearDelegates()
{
	Super::ClearDelegates();
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
