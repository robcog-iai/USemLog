// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisLiveViewMaskHandler.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "SLSkeletalDataComponent.h"
#include "Tags.h"


// Ctor
USLVisLiveViewMaskHandler::USLVisLiveViewMaskHandler()
{
	bIsInit = false;
	bMaskMaterialsOn = false;
};


// Init
void USLVisLiveViewMaskHandler::Init()
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
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material! NOT initalized.."), *FString(__func__), __LINE__);
			return;
		}

		// Save the original color materials of the static meshes, and create a for each mesh a mask material
		SetupStaticMeshes();

		// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
		SetupSkeletalMeshes();

		bIsInit = true;
	}
};

// Save the original color materials of the static meshes, and create a for each mesh a mask material
void USLVisLiveViewMaskHandler::SetupStaticMeshes()
{	
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{
		if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
		{
			// Store the original materials
			OriginalMaterials.Emplace(SMC, SMC->GetMaterials());

			// The static mesh has one mask color, check if the hex value is stored by the actor or component
			FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");
			if (!ColorHex.IsEmpty())
			{
				FColor SemColor(FColor::FromHex(ColorHex));
				UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
				MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
			}
			else
			{
				ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					FColor SemColor(FColor::FromHex(ColorHex));
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
					MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask, ignoring.."), *FString(__func__), __LINE__, *SMAItr->GetName());
					IgnoredMeshes.Emplace(SMC);
				}
			}
		}
	}
}

// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
void USLVisLiveViewMaskHandler::SetupSkeletalMeshes()
{
	// Iterate all skeletal meshes and check if they have a skeletal data component
	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{
		if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
		{
			// Store the original materials
			OriginalMaterials.Emplace(SkMC, SkMC->GetMaterials());

			// Get the mask materials of the bones
			if (USLSkeletalDataComponent* SkelDataComp = Cast<USLSkeletalDataComponent>(
				SkMAItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass())))
			{
				// Make sure the owner semantic data is set
				if (SkelDataComp->Init())
				{
					const FString OwnerId = SkelDataComp->OwnerSemanticData.Id;
					const FString OwnerClass = SkelDataComp->OwnerSemanticData.Class;

					// Skeletal mesh mask materials, init with mesh and the default materials
					FSLVisSkelMasks SkelMeshMasks(SkMC, DefaultMaskMaterial);

					// Iterate the bones and add the mask materials
					for (const auto& BoneDataPair : SkelDataComp->SemanticBonesData)
					{
						const FString BoneColorHex = BoneDataPair.Value.VisualMask;
						const FString BoneClass = BoneDataPair.Value.Class;

						FColor SemColor(FColor::FromHex(BoneColorHex));
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));

						SkelMeshMasks.MaskMaterials.Emplace(BoneDataPair.Value.MaskMaterialIndex, DynamicMaskMaterial);
					}

					SkelMaskMaterials.Emplace(SkelMeshMasks);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot init skeletal data component.. (this should not happen)"), *FString(__func__), __LINE__);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no USLSkeletalDataComponent, ignoring.."), *FString(__func__), __LINE__, *SkMAItr->GetName());
				IgnoredMeshes.Emplace(SkMC);
			}
		}
	}
}

// Apply mask materials
bool USLVisLiveViewMaskHandler::ApplyMaskMaterials()
{
	if (bIsInit)
	{
		for (const auto& MeshMatPair : MaskMaterials)
		{
			for (int32 Idx = 0; Idx < MeshMatPair.Key->GetNumMaterials(); ++Idx)
			{
				MeshMatPair.Key->SetMaterial(Idx, MeshMatPair.Value);
			}
		}
		for (auto& SkelMat : SkelMaskMaterials)
		{
			SkelMat.ApplyMaterials();
		}
		for (auto& Mesh : IgnoredMeshes)
		{
			for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
			{
				Mesh->SetMaterial(Idx, DefaultMaskMaterial);
			}
		}
		bMaskMaterialsOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLVisLiveViewMaskHandler::ApplyOriginalMaterials()
{
	if (bIsInit)
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
		bMaskMaterialsOn = false;
		return true;
	}
	return false;
}

// Toggle between the mask and original materials
bool USLVisLiveViewMaskHandler::ToggleMaterials()
{
	if (bMaskMaterialsOn)
	{
		return USLVisLiveViewMaskHandler::ApplyOriginalMaterials();
	}
	else
	{
		return USLVisLiveViewMaskHandler::ApplyMaskMaterials();
	}
}
