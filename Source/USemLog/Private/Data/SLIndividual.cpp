// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividual.h"
#include "Utils/SLTagIO.h"

// Ctor
USLIndividual::USLIndividual()
{
}

void USLIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Super::Init();
}

// Save data to tag
bool USLIndividual::SaveToTag(bool bOverwrite)
{
	if (!Super::SaveToTag(bOverwrite))
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
bool USLIndividual::LoadFromTag(bool bOverwrite)
{
	if (!Super::LoadFromTag(bOverwrite))
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

// All properties are set for runtime
bool USLIndividual::IsRuntimeReady() const
{
	if (!Super::IsRuntimeReady())
	{
		return false;
	}

	return !Id.IsEmpty() && !Class.IsEmpty();
}
