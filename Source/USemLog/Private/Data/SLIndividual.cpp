// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividual.h"
#include "Utils/SLTagIO.h"

// Save data to tag
bool USLIndividual::SaveToTag(bool bOverwrite)
{
	if (!Super::SaveToTag(bOverwrite))
	{
		return false;
	}

	if (!Id.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "Id", Id, bOverwrite);
	}

	if (!Class.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "Class", Class, bOverwrite);
	}

	return true;
}

// Load data from owners tag
bool USLIndividual::LoadFromTag(bool bOverwrite)
{
	if (!Super::LoadFromTag(bOverwrite))
	{
		return false;
	}
	
	if (Id.IsEmpty() || bOverwrite)
	{
		Id = FSLTagIO::GetValue(SemOwner, TagTypeConst, "Id");
	}

	if (Class.IsEmpty() || bOverwrite)
	{
		Class = FSLTagIO::GetValue(SemOwner, TagTypeConst, "Class");
	}

	return true;
}

// All properties are set for runtime
bool USLIndividual::IsRuntimeReady() const
{
	if (!Super::IsRuntimeReady())
	{
		return false;
	}

	return !Id.IsEmpty() && !Class.IsEmpty();
}
