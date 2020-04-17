// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividual.h"
#include "Utils/SLTagIO.h"

// Ctor
USLIndividual::USLIndividual()
{
	bIsInitPrivate = false;
	bIsLoadedPrivate = false;
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
		bIsInitPrivate = false;
	}

	if (IsInit())
	{
		return true;
	}

	if (!Super::Init(bForced))
	{
		return false;
	}

	bIsInitPrivate = InitImpl();
	return bIsInitPrivate;
}

// Check if individual is initialized
bool USLIndividual::IsInit() const
{
	return bIsInitPrivate && Super::IsInit();
}

// Load semantic data
bool USLIndividual::Load(bool bForced)
{
	if (bForced)
	{
		bIsLoadedPrivate = false;
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		return false;
	}

	if (!Super::Load(bForced))
	{
		return false;
	}

	bIsLoadedPrivate = LoadImpl();
	return bIsLoadedPrivate;
}

// Check if semantic data is succesfully loaded
bool USLIndividual::IsLoaded() const
{
	return bIsLoadedPrivate && Super::IsLoaded();
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
		Id = FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Id");
	}

	if (Class.IsEmpty() || bOverwrite)
	{
		Class = FSLTagIO::GetValue(SemanticOwner, TagTypeConst, "Class");
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
