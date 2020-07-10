// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLVirtualBoneIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Components/SkeletalMeshComponent.h"

// Utils
#include "Individuals/SLIndividualUtils.h"
#include "Utils/SLTagIO.h"

// Ctor
USLVirtualBoneIndividual::USLVirtualBoneIndividual()
{
	BoneIndex = INDEX_NONE;
	bIsPreInit = false;
}

// Called before destroying the object.
void USLVirtualBoneIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLVirtualBoneIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	//Init();
}

// Set the parameters required when initalizing the individual
bool USLVirtualBoneIndividual::PreInit(int32 NewBoneIndex, bool bReset)
{
	if (bReset)
	{
		bIsPreInit = false;
	}

	if (IsPreInit())
	{
		return true;
	}

	BoneIndex = NewBoneIndex;
	ImportTagType += "Bone" + FString::FromInt(BoneIndex);
	bIsPreInit = true;
	return true;	
}

// Set pointer to the semantic owner
bool USLVirtualBoneIndividual::Init(bool bReset)
{
	if (!IsPreInit())
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot init individual %s, pre init need to be called right after creation.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}

	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init() && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLVirtualBoneIndividual::Load(bool bReset, bool bTryImport)
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}

// Calculate the current bone transform
bool USLVirtualBoneIndividual::CacheCurrentBoneTransform()
{
	if (IsInit())
	{
		CachedTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLVirtualBoneIndividual::CalcDefaultClassValue() const
{
	return GetTypeName();
}

// Clear all values of the individual
void USLVirtualBoneIndividual::InitReset()
{
	SetIsInit(false);
	ClearDelegateBounds();
	Super::InitReset();
}

// Clear all data of the individual
void USLVirtualBoneIndividual::LoadReset()
{
	Super::LoadReset();
}

// Set pointer to parent actor
bool USLVirtualBoneIndividual::SetParentActor()
{
	if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
	{
		if (UActorComponent* AC = Cast<UActorComponent>(SkI->GetOuter()))
		{
			if (AActor* CompOwner = Cast<AActor>(AC->GetOuter()))
			{
				ParentActor = CompOwner;
				return true;
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's outer should be a skeletal individual.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
	}
	return false;
}

// Clear any bound delegates (called when init is reset)
void USLVirtualBoneIndividual::ClearDelegateBounds()
{
}

// Private init implementation
bool USLVirtualBoneIndividual::InitImpl()
{
	// Make sure the visual mesh is set
	if (HasValidSkeletalMesh() || SetSkeletalMesh())
	{
		// TODO set parent and child bone individual
		return true;
	}
	return false;
}

// Private load implementation
bool USLVirtualBoneIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Check if the static mesh component is set
bool USLVirtualBoneIndividual::HasValidSkeletalMesh() const
{
	return SkeletalMeshComponent && SkeletalMeshComponent->IsValidLowLevel() && !SkeletalMeshComponent->IsPendingKill();
}

// Set sekeletal mesh
bool USLVirtualBoneIndividual::SetSkeletalMesh()
{
	// Outer should be the skeletal individual
	if (USLSkeletalIndividual* SkI = Cast<USLSkeletalIndividual>(GetOuter()))
	{
		if (SkI->HasValidSkeletalMesh())
		{
			SkeletalMeshComponent = SkI->SkeletalMeshComponent;
			return true;
		}
	}
	return false;
}

