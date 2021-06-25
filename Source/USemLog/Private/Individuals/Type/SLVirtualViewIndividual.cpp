// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLVirtualViewIndividual.h"

// Ctor
USLVirtualViewIndividual::USLVirtualViewIndividual()
{
}

// Called before destroying the object.
void USLVirtualViewIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLVirtualViewIndividual::Init(bool bReset)
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
bool USLVirtualViewIndividual::Load(bool bReset, bool bTryImport)
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
FString USLVirtualViewIndividual::CalcDefaultClassValue()
{
	if (IsInit())
	{
		if (IsAttachedToAnotherIndividual())
		{
			if (AttachedToIndividual->IsClassValueSet())
			{
				return AttachedToIndividual->GetClassValue() + "VirtualView";
			}
		}
	}
	return GetTypeName();
}

// Private init implementation
bool USLVirtualViewIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLVirtualViewIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLVirtualViewIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLVirtualViewIndividual::LoadReset()
{
	SetIsLoaded(false);
}