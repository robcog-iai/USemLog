// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBaseConstraintIndividual.h"
#include "PhysicsEngine/ConstraintInstance.h"

// Utils 
#include "Individuals/SLIndividualUtils.h"

// Ctor
USLBaseConstraintIndividual::USLBaseConstraintIndividual()
{
	ConstraintInstance = nullptr;
	ChildIndividual = nullptr;
	ParentIndividual = nullptr;
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

// Check if the constraint instance is valid
bool USLBaseConstraintIndividual::HasValidConstraintInstance() const
{
	return ConstraintInstance != nullptr;
}

// Check if the child individual is valid 
bool USLBaseConstraintIndividual::HasValidChildIndividual() const
{
	return ChildIndividual && ChildIndividual->IsValidLowLevel() && !ChildIndividual->IsPendingKill();
}

// Check if the parent individual is valid
bool USLBaseConstraintIndividual::HasValidParentIndividual() const
{
	return ParentIndividual && ParentIndividual->IsValidLowLevel() && !ParentIndividual->IsPendingKill();
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
	ConstraintInstance = nullptr;
	ChildIndividual = nullptr;
	ParentIndividual = nullptr;
	SetIsInit(false);
}

// Clear all data of the individual
void USLBaseConstraintIndividual::LoadReset()
{
	SetIsLoaded(false);
}
