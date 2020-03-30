// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLVisualIndividual.h"
#include "Utils/SLTagIO.h"

// Save data to owners tag
bool USLVisualIndividual::SaveToTag(bool bOverwrite)
{
	if (!Super::SaveToTag(bOverwrite))
	{
		return false;
	}

	if (!VisualMask.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "VisualMask", VisualMask, bOverwrite);
	}

	if (!CalibratedVisualMask.IsEmpty())
	{
		FSLTagIO::AddKVPair(SemOwner, TagTypeConst, "CalibratedVisualMask", CalibratedVisualMask, bOverwrite);
	}

	return true;
}

// Load data from owners tag
bool USLVisualIndividual::LoadFromTag(bool bOverwrite)
{
	if (!Super::LoadFromTag(bOverwrite))
	{
		return false;
	}

	if (VisualMask.IsEmpty() || bOverwrite)
	{
		VisualMask = FSLTagIO::GetValue(SemOwner, TagTypeConst, "VisualMask");
	}

	if (CalibratedVisualMask.IsEmpty() || bOverwrite)
	{
		CalibratedVisualMask = FSLTagIO::GetValue(SemOwner, TagTypeConst, "CalibratedVisualMask");
	}

	return true;
}

// All properties are set for runtime
bool USLVisualIndividual::IsRuntimeReady() const
{
	if (!Super::IsRuntimeReady())
	{
		return false;
	}

	return !VisualMask.IsEmpty() && !CalibratedVisualMask.IsEmpty();
}
