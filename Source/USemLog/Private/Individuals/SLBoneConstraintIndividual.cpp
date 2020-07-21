// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBoneConstraintIndividual.h"

// Utils 
#include "Individuals/SLIndividualUtils.h"

// Ctor
USLBoneConstraintIndividual::USLBoneConstraintIndividual()
{
	bIsPreInit = false;
}

// Called before destroying the object.
void USLBoneConstraintIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set the parameters required when initalizing the individual
bool USLBoneConstraintIndividual::PreInit(int32 NewConstraintIndex, bool bReset)
{
	if (bReset)
	{
		bIsPreInit = false;
	}

	if (IsPreInit())
	{
		return true;
	}

	ConstraintIndex = NewConstraintIndex;
	TagType += "BoneConstraint" + FString::FromInt(ConstraintIndex);
	bIsPreInit = true;
	return true;
}


// Set pointer to the semantic owner
bool USLBoneConstraintIndividual::Init(bool bReset)
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
bool USLBoneConstraintIndividual::Load(bool bReset, bool bTryImport)
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
FString USLBoneConstraintIndividual::CalcDefaultClassValue()
{
	// if (IsInit())
	// {
		// if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(ParentActor))
		// {
			// if (UPhysicsConstraintComponent* PCC = PCA->GetConstraintComp())
			// {
				// if (PCC->ConstraintInstance.GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
					// PCC->ConstraintInstance.GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
					// PCC->ConstraintInstance.GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
				// {
					// return "LinearJoint";
				// }
				// else if (PCC->ConstraintInstance.GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
					// PCC->ConstraintInstance.GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
					// PCC->ConstraintInstance.GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
				// {
					// return "RevoluteJoint";
				// }
				// else
				// {
					// return "FixedJoint";
				// }
			// }
		// }
	// }
	return GetTypeName();
}

// Private init implementation
bool USLBoneConstraintIndividual::InitImpl()
{
	if (!IsPreInit())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot init individual %s, pre init need to be called right after creation.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	// if (HasValidConstaintEntities())
	// {
		// return true;
	// }

	// if (APhysicsConstraintActor* PCA = Cast<APhysicsConstraintActor>(ParentActor))
	// {
		// if (UPhysicsConstraintComponent* PCC = PCA->GetConstraintComp())
		// {
			// /* Actor and individual 1 */
			// if (PCC->ConstraintActor1)
			// {
				// ConstraintActor1 = PCC->ConstraintActor1;
				// if (USLBaseIndividual* BI1 = FSLIndividualUtils::GetIndividualObject(ConstraintActor1))
				// {
					// ConstraintIndividual1 = BI1;
				// }
				// else
				// {
					// UE_LOG(LogTemp, Error, TEXT("%s::%d %s constraint components ConstraintActor1 does not have an individual, init failed.."),
						// *FString(__FUNCTION__), __LINE__, *GetFullName());
					// return false;
				// }
			// }
			// else
			// {
				// UE_LOG(LogTemp, Error, TEXT("%s::%d %s constraint components ConstraintActor1 not set, init failed.."),
					// *FString(__FUNCTION__), __LINE__, *GetFullName());
				// return false;
			// }

			// /* Actor and individual 2 */
			// if (PCC->ConstraintActor2)
			// {
				// ConstraintActor2 = PCC->ConstraintActor2;
				// if (USLBaseIndividual* BI2 = FSLIndividualUtils::GetIndividualObject(ConstraintActor2))
				// {
					// ConstraintIndividual2 = BI2;
				// }
				// else
				// {
					// UE_LOG(LogTemp, Error, TEXT("%s::%d %s constraint components ConstraintActor2 does not have an individual, init failed.."),
						// *FString(__FUNCTION__), __LINE__, *GetFullName());
					// return false;
				// }
			// }
			// else
			// {
				// UE_LOG(LogTemp, Error, TEXT("%s::%d %s constraint components ConstraintActor2 not set, init failed.."),
					// *FString(__FUNCTION__), __LINE__, *GetFullName());
				// return false;
			// }
		// }
	// }
	// else
	// {
		// UE_LOG(LogTemp, Error, TEXT("%s::%d %s parent actor is not a physics constraint actor, this should not happen, init failed.."),
			// *FString(__FUNCTION__), __LINE__, *GetFullName());
		// return false;
	// }

	// return HasValidConstaintEntities();
	return true;
}

// Private load implementation
bool USLBoneConstraintIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLBoneConstraintIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLBoneConstraintIndividual::LoadReset()
{
	SetIsLoaded(false);
}
