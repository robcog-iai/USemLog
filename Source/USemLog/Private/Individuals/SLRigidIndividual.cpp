// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLRigidIndividual.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Ctor
USLRigidIndividual::USLRigidIndividual()
{
	VisualMeshComponent = nullptr;
}

// Called before destroying the object.
void USLRigidIndividual::BeginDestroy()
{
	Super::BeginDestroy();
}

// Create and set the dynamic material, the owners visual component
void USLRigidIndividual::PostInitProperties()
{
	Super::PostInitProperties();
	Init();
}

// Set pointer to the semantic owner
bool USLRigidIndividual::Init(bool bReset)
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
bool USLRigidIndividual::Load(bool bReset, bool bTryImport)
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
bool USLRigidIndividual::ApplyMaskMaterials(bool bPrioritizeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		for (int32 MatIdx = 0; MatIdx < VisualMeshComponent->GetNumMaterials(); ++MatIdx)
		{
			VisualMeshComponent->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
		}
		bIsMaskMaterialOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLRigidIndividual::ApplyOriginalMaterials()
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
			VisualMeshComponent->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Clear all values of the individual
void USLRigidIndividual::InitReset()
{
	LoadReset();
	Super::InitReset();
	VisualMeshComponent = nullptr;
	SetIsInit(false);
	ClearDelegateBounds();
}

// Clear all data of the individual
void USLRigidIndividual::LoadReset()
{
	Super::LoadReset();
}


// Private init implementation
bool USLRigidIndividual::InitImpl()
{
	if (!HasValidVisualMesh())
	{
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(ParentActor))
		{
			if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
			{
				VisualMeshComponent = SMC;
				OriginalMaterials = SMC->GetMaterials();
				return true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no StaticMeshComponent, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *GetFullName());
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s SemanticOwner is not a StaticMeshActor, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}
	return false;
}

// Private load implementation
bool USLRigidIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Check if the static mesh component is set
bool USLRigidIndividual::HasValidVisualMesh() const
{
	return VisualMeshComponent && VisualMeshComponent->IsValidLowLevel() && !VisualMeshComponent->IsPendingKill();
}
