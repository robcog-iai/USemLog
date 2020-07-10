// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLVirtualViewIndividual.h"


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

// Create and set the dynamic material, the owners visual component
void USLVirtualViewIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	//Init();
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

	SetIsInit(Super::Init() && InitImpl());
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

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}

// Get class name, virtual since each invidiual type will have different name
FString USLVirtualViewIndividual::CalcDefaultClassValue() const
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

// Clear all values of the individual
void USLVirtualViewIndividual::InitReset()
{
	SetIsInit(false);
	ClearDelegateBounds();
	Super::InitReset();
}

// Clear all data of the individual
void USLVirtualViewIndividual::LoadReset()
{
	Super::LoadReset();
}

// Clear any bound delegates (called when init is reset)
void USLVirtualViewIndividual::ClearDelegateBounds()
{
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
