// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisMaskVisualizer.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Tags.h"

// Ctor
USLVisMaskVisualizer::USLVisMaskVisualizer()
{
	bIsInit = false;
	bAreMaskMaterialsOn = false;
};

// Dtor
USLVisMaskVisualizer::~USLVisMaskVisualizer()
{
}

// Init
void USLVisMaskVisualizer::Init()
{
	if (!bIsInit)
	{
		// Load default mask material
		DefaultMaskMaterial = LoadObject<UMaterial>(this,
			TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
		if (DefaultMaskMaterial)
		{
			DefaultMaskMaterial->bUsedWithStaticLighting = true;
			DefaultMaskMaterial->bUsedWithSkeletalMesh = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material! NOT initalized.."),
				TEXT(__FUNCTION__), __LINE__);
			return;
		}

		// Cache original materials, and create mask materials for static meshes
		for (TActorIterator<AStaticMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
		{
			if (UStaticMeshComponent* SMC = SkMAItr->GetStaticMeshComponent())
			{
				OriginalMaterials.Emplace(SMC, SMC->GetMaterials());

				FString ColorHex = FTags::GetValue(*SkMAItr, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
					MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
				}
				else
				{
					ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
					if (!ColorHex.IsEmpty())
					{
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
						MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
					}
					else
					{
						IgnoredMeshes.Emplace(SMC);
					}
				}
			}
		}

		// TODO include bone masks
		// Cache original materials, and create mask materials for static meshes
		for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
		{
			if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
			{
				OriginalMaterials.Emplace(SkMC, SkMC->GetMaterials());

				FString ColorHex = FTags::GetValue(*SkMAItr, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
					MaskMaterials.Emplace(SkMC, DynamicMaskMaterial);
				}
				else
				{
					ColorHex = FTags::GetValue(SkMC, "SemLog", "VisMask");
					if (!ColorHex.IsEmpty())
					{
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(FColor::FromHex(ColorHex)));
						MaskMaterials.Emplace(SkMC, DynamicMaskMaterial);
					}
					else
					{
						IgnoredMeshes.Emplace(SkMC);
					}
				}
			}
		}
		bIsInit = true;
	}
};

// Apply mask materials
void USLVisMaskVisualizer::ApplyMaskMaterials()
{
	for (const auto& MeshMatPair : MaskMaterials)
	{
		for (int32 Idx = 0; Idx < MeshMatPair.Key->GetNumMaterials(); ++Idx)
		{
			MeshMatPair.Key->SetMaterial(Idx, MeshMatPair.Value);
		}
	}
	for (auto& Mesh : IgnoredMeshes)
	{
		for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
		{
			Mesh->SetMaterial(Idx, DefaultMaskMaterial);
		}
	}
	bAreMaskMaterialsOn = true;
}

// Apply original materials
void USLVisMaskVisualizer::ApplyOriginalMaterials()
{
	for (const auto& MeshMatPair : OriginalMaterials)
	{
		int32 Idx = 0;
		for (auto& Mat : MeshMatPair.Value)
		{
			MeshMatPair.Key->SetMaterial(Idx, Mat);
			Idx++;
		}
	}
	bAreMaskMaterialsOn = false;
}

// Toggle between the mask and original materials
void USLVisMaskVisualizer::Toggle()
{
	if (bAreMaskMaterialsOn)
	{
		USLVisMaskVisualizer::ApplyOriginalMaterials();
	}
	else
	{
		USLVisMaskVisualizer::ApplyMaskMaterials();
	}
}