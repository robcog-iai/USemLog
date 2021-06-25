// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLBaseConstraintIndividual.h"
#include "PhysicsEngine/ConstraintInstance.h"

// Utils 
#include "Individuals/SLIndividualUtils.h"

// Ctor
USLBaseConstraintIndividual::USLBaseConstraintIndividual()
{
	ConstraintIndividual1 = nullptr;
	ConstraintIndividual2 = nullptr;
}

// Called before destroying the object.
void USLBaseConstraintIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLBaseConstraintIndividual::Init(bool bReset)
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
bool USLBaseConstraintIndividual::Load(bool bReset, bool bTryImport)
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

// Get class name, virtual since each invidiual type will have different name
FString USLBaseConstraintIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Check if the constraint1 individual is valid ('child' bone in a PhysicsAsset) 
bool USLBaseConstraintIndividual::HasValidConstraint1Individual() const
{
	return ConstraintIndividual1 && ConstraintIndividual1->IsValidLowLevel() && !ConstraintIndividual1->IsPendingKill();
}

// Check if the constraint1 individual is valid ('parent' bone in a PhysicsAsset)
bool USLBaseConstraintIndividual::HasValidConstraint2Individual() const
{
	return ConstraintIndividual2 && ConstraintIndividual2->IsValidLowLevel() && !ConstraintIndividual2->IsPendingKill();
}

// Private init implementation
bool USLBaseConstraintIndividual::InitImpl()
{
	// The class cannot set its data without having the specialization set
	return true;
}

// Private load implementation
bool USLBaseConstraintIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLBaseConstraintIndividual::InitReset()
{
	ConstraintIndividual1 = nullptr;
	ConstraintIndividual2 = nullptr;
	SetIsInit(false);
}

// Clear all data of the individual
void USLBaseConstraintIndividual::LoadReset()
{
	SetIsLoaded(false);
}
