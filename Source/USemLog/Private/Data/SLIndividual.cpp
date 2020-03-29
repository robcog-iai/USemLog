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
		FSLTagIO::AddKVPair(Owner, "SemLog", "Id", Id, bOverwrite);
	}

	if (!Class.IsEmpty())
	{
		FSLTagIO::AddKVPair(Owner, "SemLog", "Class", Class, bOverwrite);
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
		Id = FSLTagIO::GetValue(Owner, "SemLog", "Id");
	}

	if (Class.IsEmpty() || bOverwrite)
	{
		Class = FSLTagIO::GetValue(Owner, "SemLog", "Class");
	}

	return true;
}
