// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLBoneConstraintIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
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
bool USLBoneConstraintIndividual::UpdateCachedPose(float Tolerance, FTransform* OutPose)
{
	if (IsInit())
	{
		if (HasValidConstraint1Individual())
		{
			bool bRetVal = ConstraintIndividual1->UpdateCachedPose(Tolerance, &CachedPose);
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return bRetVal;
		}
		else if (HasValidConstraint2Individual())
		{
			bool bRetVal = ConstraintIndividual2->UpdateCachedPose(Tolerance, &CachedPose);
			if (OutPose != nullptr)
			{
				*OutPose = CachedPose;
			}
			return bRetVal;
		}
	}

	if (!CachedPose.Equals(FTransform::Identity, Tolerance))
	{
		CachedPose = FTransform::Identity;
		if (OutPose != nullptr)
		{
			*OutPose = CachedPose;
		}
		return true;
	}
	else
	{
		if (OutPose != nullptr)
		{
			*OutPose = CachedPose;
		}
		return false;
	}
}


// Get class name, virtual since each invidiual type will have different name
FString USLBoneConstraintIndividual::CalcDefaultClassValue()
{
	if (HasValidSkeletalMeshComponent() || SetSkeletalMeshComponent())
	{
		if(FConstraintInstance * CI = GetConstraintInstance())
		{
			if (CI->GetLinearXMotion() != ELinearConstraintMotion::LCM_Locked ||
				CI->GetLinearYMotion() != ELinearConstraintMotion::LCM_Locked ||
				CI->GetLinearZMotion() != ELinearConstraintMotion::LCM_Locked)
			{
				return "BoneLinearJoint";
			}
			else if (CI->GetAngularSwing1Motion() != EAngularConstraintMotion::ACM_Locked ||
				CI->GetAngularSwing2Motion() != EAngularConstraintMotion::ACM_Locked ||
				CI->GetAngularTwistMotion() != EAngularConstraintMotion::ACM_Locked)
			{
				return "BoneRevoluteJoint";
			}
			else
			{
				return "BoneFixedJoint";
			}
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
		if (FConstraintInstance* CI = GetConstraintInstance())
		{
			const FName ConstraintBoneName = CI->ConstraintBone1;
			int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(ConstraintBoneName);
			if (BoneIndex != INDEX_NONE)
			{
				if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
				{
					if (USLBaseIndividual* BI = SkI->GetBoneIndividual(BoneIndex))
					{
						ConstraintIndividual1 = BI;
						return true;
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
		if (FConstraintInstance* CI = GetConstraintInstance())
		{
			const FName ConstraintBoneName = CI->ConstraintBone2;
			int32 BoneIndex = SkeletalMeshComponent->GetBoneIndex(ConstraintBoneName);
			if (BoneIndex != INDEX_NONE)
			{
				if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
				{
					if (USLBaseIndividual* BI = SkI->GetBoneIndividual(BoneIndex))
					{
						ConstraintIndividual2 = BI;
						return true;
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
	}
	return false;
}

// Get the constraint instance of the individual
FConstraintInstance* USLBoneConstraintIndividual::GetConstraintInstance() const
{
	if (SkeletalMeshComponent->Constraints.IsValidIndex(ConstraintIndex))
	{
		return SkeletalMeshComponent->Constraints[ConstraintIndex];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot access %s's constraint instance[Index=%ld/Total=%ld].."),
			*FString(__FUNCTION__), __LINE__, *GetFullName(), ConstraintIndex, SkeletalMeshComponent->Constraints.Num());
	}
	return nullptr;
}

// Check if the constraint index is valid
bool USLBoneConstraintIndividual::HasValidConstraintIndex() const
{
	if (HasValidSkeletalMeshComponent())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d HasValidSkeletalMeshComponent gud=%ld , Constraints.Num()=%ld"),
			*FString(__FUNCTION__), __LINE__, ConstraintIndex, SkeletalMeshComponent->Constraints.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d HasValidSkeletalMeshComponent NOTgud=%ld "), *FString(__FUNCTION__), __LINE__, ConstraintIndex);
	}
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
