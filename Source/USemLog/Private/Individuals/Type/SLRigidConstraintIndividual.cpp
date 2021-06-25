// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLRigidConstraintIndividual.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Utils 
#include "Individuals/SLIndividualUtils.h"

// Ctor
USLRigidConstraintIndividual::USLRigidConstraintIndividual()
{
	ConstraintActor1 = nullptr;
	ConstraintActor2 = nullptr;
}

// Called before destroying the object.
void USLRigidConstraintIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLRigidConstraintIndividual::Init(bool bReset)
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
bool USLRigidConstraintIndividual::Load(bool bReset, bool bTryImport)
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
FString USLRigidConstraintIndividual::CalcDefaultClassValue()
{
	if (HasValidConstraintComponent() || SetConstraintComponent())
	{
		if (FConstraintInstance* CI = GetConstraintInstance())
		{
			if (CI->GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
				CI->GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
				CI->GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
			{
				return "LinearJoint";
			}
			else if (CI->GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
				CI->GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
				CI->GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
			{
				return "RevoluteJoint";
			}
			else
			{
				return "FixedJoint";
			}
		}
	}
	return GetTypeName();
}


// Set the child individual object
bool USLRigidConstraintIndividual::SetConstraint1Individual()
{
	if (HasValidConstraint1Individual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint1 individual is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidConstraintActor1())
	{
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(ConstraintActor1))
		{
			ConstraintIndividual1 = BI;
			return HasValidConstraint1Individual();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor1 %s has no individual object.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *ConstraintActor1->GetName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's cannot set child individual without a valid ConstraintActor1 .."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Set the parent individual object
bool USLRigidConstraintIndividual::SetConstraint2Individual()
{	
	if (HasValidConstraint2Individual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint2 individual is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidConstraintActor2())
	{
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(ConstraintActor2))
		{
			ConstraintIndividual2 = BI;
			return HasValidConstraint1Individual();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor2 %s has no individual object.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *ConstraintActor2->GetName());
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's cannot set child individual without a valid ConstraintActor2 .."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Get the constraint instance of the individual
FConstraintInstance* USLRigidConstraintIndividual::GetConstraintInstance() const
{
	return &ConstraintComponent->ConstraintInstance;
}

// Check if the constraint component is valid
bool USLRigidConstraintIndividual::HasValidConstraintComponent() const
{
	return ConstraintComponent && ConstraintComponent->IsValidLowLevel() && !ConstraintComponent->IsPendingKill();
}

// Set the constraint component
bool USLRigidConstraintIndividual::SetConstraintComponent()
{
	if (HasValidConstraintComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint component is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidParentActor())
	{
		if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(ParentActor))
		{
			if (UPhysicsConstraintComponent* PCC = PCA->GetConstraintComp())
			{
				ConstraintComponent = PCC;
				return HasValidConstraintComponent();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's constraint component cannot be set without a physics constraint component.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's constraint component cannot be set without a physics constraint actor parent.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's constraint component cannot be set without a valid parent actor.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if the constraint actor1 is valid
bool USLRigidConstraintIndividual::HasValidConstraintActor1() const
{
	return ConstraintActor1 && ConstraintActor1->IsValidLowLevel() && !ConstraintActor1->IsPendingKill();
}

// Set constraint actor1 (child)
bool USLRigidConstraintIndividual::SetConstraintActor1()
{
	if (HasValidConstraintActor1())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's ConstraintActor1 is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidConstraintComponent())
	{
		ConstraintActor1 = ConstraintComponent->ConstraintActor1;
		if (HasValidConstraintActor1())
		{
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor1 cannot be set without a valid ConstraintActor1 member.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor1 cannot be set without a valid constraint component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Check if the constraint actor2 is valid
bool USLRigidConstraintIndividual::HasValidConstraintActor2() const
{
	return ConstraintActor2 && ConstraintActor2->IsValidLowLevel() && !ConstraintActor2->IsPendingKill();
}

// Set constraint actor2 (parent)
bool USLRigidConstraintIndividual::SetConstraintActor2()
{
	if (HasValidConstraintActor2())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's ConstraintActor2 is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidConstraintComponent())
	{
		ConstraintActor2 = ConstraintComponent->ConstraintActor2;
		if (HasValidConstraintActor2())
		{
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor2 cannot be set without a valid ConstraintActor2 member.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ConstraintActor2 cannot be set without a valid constraint component.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Private init implementation
bool USLRigidConstraintIndividual::InitImpl()
{
	if (HasValidConstraintComponent() || SetConstraintComponent())
	{
		bool bMembersSet = true;
		if (HasValidConstraintActor1() || SetConstraintActor1())
		{
			if (!(HasValidConstraint1Individual() || SetConstraint1Individual()))
			{
				bMembersSet = false;
			}
		}
		else
		{
			bMembersSet = false;
		}

		if (HasValidConstraintActor2() || SetConstraintActor2())
		{
			if (!(HasValidConstraint1Individual() || SetConstraint1Individual()))
			{
				bMembersSet = false;
			}
		}
		else
		{
			bMembersSet = false;
		}
		return bMembersSet;
	}
	return false;
}

// Private load implementation
bool USLRigidConstraintIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLRigidConstraintIndividual::InitReset()
{
	ConstraintActor1 = nullptr;
	ConstraintActor2 = nullptr;
	ConstraintComponent = nullptr;
	SetIsInit(false);
}

// Clear all data of the individual
void USLRigidConstraintIndividual::LoadReset()
{
	SetIsLoaded(false);
}
