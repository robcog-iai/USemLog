// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLVirtualGazeOriginIndividual.h"

// Ctor
USLVirtualGazeOriginIndividual::USLVirtualGazeOriginIndividual()
{
}

// Called before destroying the object.
void USLVirtualGazeOriginIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLVirtualGazeOriginIndividual::Init(bool bReset)
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
bool USLVirtualGazeOriginIndividual::Load(bool bReset, bool bTryImport)
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

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

// Get class name, virtual since each invidiual type will have different name
FString USLVirtualGazeOriginIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLVirtualGazeOriginIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLVirtualGazeOriginIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLVirtualGazeOriginIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLVirtualGazeOriginIndividual::LoadReset()
{
	SetIsLoaded(false);
}