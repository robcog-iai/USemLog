// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/Type/SLRigidIndividual.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Ctor
USLRigidIndividual::USLRigidIndividual()
{
	StaticMeshComponent = nullptr;
}

// Called before destroying the object.
void USLRigidIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
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

	SetIsInit(Super::Init(bReset) && InitImpl());
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
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

// Apply visual mask material
bool USLRigidIndividual::ApplyMaskMaterials(bool bIncludeChildren /*= false*/)
{
	if (!IsInit())
	{
		return false;
	}

	if (!bIsMaskMaterialOn)
	{
		for (int32 MatIdx = 0; MatIdx < StaticMeshComponent->GetNumMaterials(); ++MatIdx)
		{
			StaticMeshComponent->SetMaterial(MatIdx, VisualMaskDynamicMaterial);
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
			StaticMeshComponent->SetMaterial(MatIdx, Mat);
			++MatIdx;
		}
		bIsMaskMaterialOn = false;
		return true;
	}
	return false;
}

// Get class name, virtual since each invidiual type will have different name
FString USLRigidIndividual::CalcDefaultClassValue()
{
	if(IsInit())
	{
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(ParentActor))
		{
			if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
			{
				FString ClassName = SMC->GetStaticMesh()->GetFullName();
				int32 FindCharPos = INDEX_NONE;

				 //Remove end path from full name
				ClassName.FindLastChar('.', FindCharPos);
				if (FindCharPos != INDEX_NONE)
				{
					ClassName.RemoveAt(0, FindCharPos + 1);
				}

				// Remove prefix SM_ClassName)
				if (!ClassName.RemoveFromStart(TEXT("SM_")))
				{
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s StaticMesh has no SM_ prefix in its name.."),
					//	*FString(__func__), __LINE__, *CompOwner->GetName());
				}

				// Remove any postfix (SM_ClassName_PostfixExample)
				ClassName.FindLastChar('_', FindCharPos);
				if (FindCharPos != INDEX_NONE)
				{
					ClassName.RemoveAt(FindCharPos, ClassName.Len() - FindCharPos);
				}
				return ClassName;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SMC.."),
					*FString(__func__), __LINE__, *SMA->GetName());
			}
		}
	}
	return GetTypeName();
}

// Private init implementation
bool USLRigidIndividual::InitImpl()
{
	if (HasValidStaticMesh() || SetStaticMesh())
	{
		OriginalMaterials = StaticMeshComponent->GetMaterials();
		return true;
	}
	return false;
}

// Private load implementation
bool USLRigidIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLRigidIndividual::InitReset()
{
	LoadReset();
	StaticMeshComponent = nullptr;
	SetIsInit(false);
}

// Clear all data of the individual
void USLRigidIndividual::LoadReset()
{
	SetIsLoaded(false);
}


// Check if the static mesh component is set
bool USLRigidIndividual::HasValidStaticMesh() const
{
	return StaticMeshComponent && StaticMeshComponent->IsValidLowLevel() && !StaticMeshComponent->IsPendingKill();
}

// Set the static mesh component
bool USLRigidIndividual::SetStaticMesh()
{
	if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(ParentActor))
	{
		if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			StaticMeshComponent = SMC;			
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s's parent actor is not a StaticMeshActor, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *GetFullName());
		return false;
	}
}
