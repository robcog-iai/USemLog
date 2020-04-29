// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividual.h"
#include "Utils/SLTagIO.h"

// Ctor
USLIndividual::USLIndividual()
{
	bIsInit = false;
	bIsLoaded = false;
}

void USLIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLIndividual::Init(bool bForced)
{
	if (bForced)
	{
		bIsInit = false;
	}

	if (IsInit())
	{
		return true;
	}

	if (!Super::Init(bForced))
	{
		return false;
	}

	bIsInit = InitImpl();
	return bIsInit;
}

// Check if individual is initialized
bool USLIndividual::IsInit() const
{
	return bIsInit && Super::IsInit();
}

// Load semantic data
bool USLIndividual::Load(bool bForced)
{
	if (bForced)
	{
		bIsLoaded = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bForced))
		{
			return false;
		}
	}

	if (!Super::Load(bForced))
	{
		return false;
	}

	bIsLoaded = LoadImpl();
	return bIsLoaded;
}

// Check if semantic data is succesfully loaded
bool USLIndividual::IsLoaded() const
{
	return bIsLoaded /*&& Super::IsLoaded()*/;
}

// Save data to tag
bool USLIndividual::ExportToTag(bool bOverwrite)
{
	if (!Super::ExportToTag(bOverwrite))
	{
		return false;
	}

	if (!Id.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "Id", Id, bOverwrite);
	}

	if (!Class.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemanticOwner, TagTypeConst, "Class", Class, bOverwrite);
	}

	return true;
}

// Load data from owners tag
bool USLIndividual::ImportFromTag(bool bOverwrite)
{
	if (!Super::ImportFromTag(bOverwrite))
	{
		return false;
	}
	
	if (Id.IsEmpty() || bOverwrite)
	{
		SetId(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Id"));
	}

	if (Class.IsEmpty() || bOverwrite)
	{
		SetClass(FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Class"));
	}

	return true;
}

// Private init implementation
bool USLIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLIndividual::LoadImpl()
{
	return HasId() && HasClass();
}
