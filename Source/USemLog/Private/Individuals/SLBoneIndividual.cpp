// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLBoneIndividual.h"
#include "Individuals/SLSkeletalIndividual.h"
#include "Animation/SkeletalMeshActor.h"

// Ctor
USLBoneIndividual::USLBoneIndividual()
{
}

// Called before destroying the object.
void USLBoneIndividual::BeginDestroy()
{
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLBoneIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLBoneIndividual::Init(bool bReset)
{
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
bool USLBoneIndividual::Load(bool bReset, bool bTryImport)
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
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load component individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load() && LoadImpl(bTryImport));
	return IsLoaded();
}



// Apply visual mask material
bool USLBoneIndividual::ApplyMaskMaterials(bool bPrioritizeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLBoneIndividual::ApplyOriginalMaterials()
{
	if (!IsInit())
	{
		return false;
	}

	if (bIsMaskMaterialOn)
	{
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Private init implementation
bool USLBoneIndividual::InitImpl()
{
	return false;
}

// Private load implementation
bool USLBoneIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLBoneIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLBoneIndividual::LoadReset()
{
	Super::LoadReset();
}

// Set the skeletal actor as parent
bool USLBoneIndividual::SetParentActor()
{
	// First outer is the skeletal individual, second is the component, third is the actor
	if (AActor* CompOwner = Cast<AActor>(GetOuter()->GetOuter()->GetOuter()))
	{
		// Set the parent actor
		ParentActor = CompOwner;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s, could not acess parent actor.."),
		*FString(__FUNCTION__), __LINE__, *GetFullName());
	return false;
}

