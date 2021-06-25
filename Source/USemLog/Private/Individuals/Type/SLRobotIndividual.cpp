// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLRobotIndividual.h"

// Ctor
USLRobotIndividual::USLRobotIndividual()
{
}

// Called before destroying the object.
void USLRobotIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLRobotIndividual::Init(bool bReset)
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
bool USLRobotIndividual::Load(bool bReset, bool bTryImport)
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
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

// Get all children of the individual in a newly created array
const TArray<USLBaseIndividual*> USLRobotIndividual::GetChildrenIndividuals() const
{
	TArray<USLBaseIndividual*> Children;
	return Children;
}


// Apply visual mask material
bool USLRobotIndividual::ApplyMaskMaterials(bool bIncludeChildren)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		if (bIncludeChildren)
		{
		}
		else
		{			
		}

		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLRobotIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		int32 MatIdx = 0;
		for (const auto& Mat : OriginalMaterials)
		{
			++MatIdx;
		}
		bIsMaskMaterialOn = false;				
		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLRobotIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLRobotIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLRobotIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLRobotIndividual::InitReset()
{
	LoadReset();
	SetIsInit(false);
}

// Clear all data of the individual
void USLRobotIndividual::LoadReset()
{
	SetIsLoaded(false);
}
