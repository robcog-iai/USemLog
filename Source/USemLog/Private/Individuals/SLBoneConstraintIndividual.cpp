// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBoneConstraintIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"

// Utils 
#include "Individuals/SLIndividualUtils.h"

// Ctor
USLBoneConstraintIndividual::USLBoneConstraintIndividual()
{
	bIsPreInit = false;
	ConstraintIndex = INDEX_NONE;
}

// Do any object-specific cleanup required immediately after loading an object.
void USLBoneConstraintIndividual::PostLoad()
{
	Super::PostLoad();
	if (!HasValidConstraintInstance())
	{
		if (!SetConstraintInstance())
		{
			if (IsInit())
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint instance could not be re-set at load time.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
			SetIsInit(false);
		}
	}
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

// Cache the current transform of the individual (returns true on a new value)
bool USLBoneConstraintIndividual::CalcAndCacheTransform(float Tolerance, FTransform* OutTransform)
{
	if (IsInit())
	{
		if (HasValidConstraint1Individual())
		{
			bool bRetVal = ConstraintIndividual1->CalcAndCacheTransform(Tolerance, &CachedTransform);
			if (OutTransform != nullptr)
			{
				*OutTransform = CachedTransform;
			}
			return bRetVal;
		}
		else if (HasValidConstraint2Individual())
		{
			bool bRetVal = ConstraintIndividual2->CalcAndCacheTransform(Tolerance, &CachedTransform);
			if (OutTransform != nullptr)
			{
				*OutTransform = CachedTransform;
			}
			return bRetVal;
		}
	}

	if (!CachedTransform.Equals(FTransform::Identity, Tolerance))
	{
		CachedTransform = FTransform::Identity;
		if (OutTransform != nullptr)
		{
			*OutTransform = CachedTransform;
		}
		return true;
	}
	else
	{
		if (OutTransform != nullptr)
		{
			*OutTransform = CachedTransform;
		}
		return false;
	}
}


// Get class name, virtual since each invidiual type will have different name
FString USLBoneConstraintIndividual::CalcDefaultClassValue()
{
	if (HasValidConstraintInstance() || SetConstraintInstance())
	{
		if (ConstraintInstance->GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
			ConstraintInstance->GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
			ConstraintInstance->GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
		{
			return "BoneLinearJoint";
		}
		else if (ConstraintInstance->GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
			ConstraintInstance->GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
			ConstraintInstance->GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
		{
			return "BoneRevoluteJoint";
		}
		else
		{
			return "BoneFixedJoint";
		}
	}
	return GetTypeName();
}

// Set the skeletal actor as parent
bool USLBoneConstraintIndividual::SetParentActor()
{
	if (USLSkeletalIndividual* Individual = Cast<USLSkeletalIndividual>(GetOuter()))
	{
		if (UActorComponent* AC = Cast<UActorComponent>(Individual->GetOuter()))
		{
			if (AActor* CompOwner = Cast<AActor>(AC->GetOuter()))
			{
				if (CompOwner->IsA(ASkeletalMeshActor::StaticClass()))
				{
					ParentActor = CompOwner;
					return true;
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ParentActor should be a skeletal mesh actor.."),
						*FString(__FUNCTION__), __LINE__, *GetFullName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's third outer should be the parent actor.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's second outer should be an actor component.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer should be a skeletal individual object.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Set the constraint instance
bool USLBoneConstraintIndividual::SetConstraintInstance()
{
	if (HasValidConstraintInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint instance is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}

	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		if (HasValidConstraintIndex())
		{
			ConstraintInstance = SkeletalMeshComponent->Constraints[ConstraintIndex];			
			return HasValidConstraintInstance();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint index is not valid.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	return false;
}

// Set the child individual object
bool USLBoneConstraintIndividual::SetConstraint1Individual()
{
	if (HasValidConstraint1Individual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint1 (child) individual is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}
	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		const FName ConstraintBoneName = ConstraintInstance->ConstraintBone1;
		int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(ConstraintBoneName);
		if (BoneIndex != INDEX_NONE)
		{
			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
			{
				if (USLBaseIndividual* BI = SkI->GetBoneIndividual(BoneIndex))
				{
					ConstraintIndividual1 = BI;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not find any sibling bone with BoneIndex=%ld .."),
						*FString(__FUNCTION__), __LINE__, *GetFullName(), BoneIndex);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not access child bone (ConstraintBone1=%s; BoneIndex=%ld;).."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *ConstraintBoneName.ToString(), BoneIndex);
		}
	}
	return false;
}

// Set the parent individual object
bool USLBoneConstraintIndividual::SetConstraint2Individual()
{
	if (HasValidConstraint2Individual())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's constraint2 (parent) individual is already valid.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return true;
	}
	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		const FName ConstraintBoneName = ConstraintInstance->ConstraintBone2;
		int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(ConstraintBoneName);
		if (BoneIndex != INDEX_NONE)
		{
			if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
			{
				if (USLBaseIndividual* BI = SkI->GetBoneIndividual(BoneIndex))
				{
					ConstraintIndividual2 = BI;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not find any sibling bone with BoneIndex=%ld .."),
						*FString(__FUNCTION__), __LINE__, *GetFullName(), BoneIndex);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's could not access child bone (ConstraintBone2=%s; BoneIndex=%ld;).."),
				*FString(__FUNCTION__), __LINE__, *GetFullName(), *ConstraintBoneName.ToString(), BoneIndex);
		}
	}
	return false;
}

// Check if the constraint index is valid
bool USLBoneConstraintIndividual::HasValidConstraintIndex() const
{
	return HasValidSkeletalMeshComponent()
		&& ConstraintIndex != INDEX_NONE
		&& ConstraintIndex < SkeletalMeshComponent->Constraints.Num();
}

// Check if the static mesh component is set
bool USLBoneConstraintIndividual::HasValidSkeletalMeshComponent() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set the skeletal mesh component
bool USLBoneConstraintIndividual::SetSkeletalMeshComponent()
{
	if (HasValidParentActor() || SetParentActor())
	{
		if (ASkeletalMeshActor* SMA = Cast<ASkeletalMeshActor>(ParentActor))
		{
			if (USkeletalMeshComponent* SMC = SMA->GetSkeletalMeshComponent())
			{
				SkeletalMeshComponent = SMC;
				return HasValidSkeletalMeshComponent();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ParentActor has no SkeletalMeshComponent, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's ParentActor should be a skeletal mesh actor.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s ParentActor is not set, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
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

	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		if (HasValidConstraintInstance() || SetConstraintInstance())
		{
			bool bMembersSet = true;
			if (!(HasValidConstraint1Individual() || SetConstraint1Individual()))
			{
				bMembersSet = false;
			}			

			if (!(HasValidConstraint2Individual() || SetConstraint2Individual()))
			{
				bMembersSet = false;
			}			
			return bMembersSet;
		}
	}
	return false;
}

// Private load implementation
bool USLBoneConstraintIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLBoneConstraintIndividual::InitReset()
{
	LoadReset();
	SetIsInit(false);
}

// Clear all data of the individual
void USLBoneConstraintIndividual::LoadReset()
{
	SetIsLoaded(false);
}
