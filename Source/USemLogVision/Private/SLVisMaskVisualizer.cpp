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
	bMaskMaterialsOn = false;
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
		for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
		{
			if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
			{
				OriginalMaterials.Emplace(SMC, SMC->GetMaterials());

				FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					FColor SemColor(FColor::FromHex(ColorHex));
					if (!USLVisMaskVisualizer::AddSemanticColorInfo(SemColor, ColorHex, *SMAItr))
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Color=[%s|%s] is missing semantic information.."),
							TEXT(__FUNCTION__), __LINE__, *ColorHex, *SemColor.ToString());
					}
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(SemColor));
					MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
				}
				else
				{
					ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
					if (!ColorHex.IsEmpty())
					{
						FColor SemColor(FColor::FromHex(ColorHex));
						if (!USLVisMaskVisualizer::AddSemanticColorInfo(SemColor, ColorHex, SMC))
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d Color=[%s|%s] is missing semantic information.."),
								TEXT(__FUNCTION__), __LINE__, *ColorHex, *SemColor.ToString());
						}
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(SemColor));
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
					FColor SemColor(FColor::FromHex(ColorHex));
					if (!USLVisMaskVisualizer::AddSemanticColorInfo(SemColor, ColorHex, *SkMAItr))
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Color=[%s|%s] is missing semantic information.."),
							TEXT(__FUNCTION__), __LINE__, *ColorHex, *SemColor.ToString());
					}
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(SemColor));
					MaskMaterials.Emplace(SkMC, DynamicMaskMaterial);
				}
				else
				{
					ColorHex = FTags::GetValue(SkMC, "SemLog", "VisMask");
					if (!ColorHex.IsEmpty())
					{
						FColor SemColor(FColor::FromHex(ColorHex));
						if (!USLVisMaskVisualizer::AddSemanticColorInfo(SemColor, ColorHex, SkMC))
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d Color=[%s|%s] is missing semantic information.."),
								TEXT(__FUNCTION__), __LINE__, *ColorHex, *SemColor.ToString());
						}
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor(SemColor));
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
bool USLVisMaskVisualizer::ApplyMaskMaterials()
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
bool USLVisMaskVisualizer::ApplyOriginalMaterials()
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
bool USLVisMaskVisualizer::Toggle()
{
	if (bMaskMaterialsOn)
	{
		return USLVisMaskVisualizer::ApplyOriginalMaterials();
	}
	else
	{
		return USLVisMaskVisualizer::ApplyMaskMaterials();
	}
}

// Get semantic objects from view, true if succeeded
bool USLVisMaskVisualizer::GetSemanticObjectsFromView(const TArray<FColor>& InBitmap, TArray<FSLVisSemanticColorInfo>& OutSemColorsInfo)
{
	if (bMaskMaterialsOn)
	{
		// Get all unique colors from image
		TSet<FColor> UniqueColors;
		for (const auto& Color : InBitmap)
		{
			if (!UniqueColors.Contains(Color))
			{
				UniqueColors.Add(Color);
			}
		}

		// Iterate unique colors and add the semantic data to the output array
		for (const auto& Color : UniqueColors)
		{
			if (SemanticColorsInfo.Contains(Color))
			{
				OutSemColorsInfo.Emplace(SemanticColorsInfo[Color]);
			}
			else
			{
				//UE_LOG(LogTemp, Error, TEXT("%s::%d Color=[%s] is missing semantic information.."),
				//	TEXT(__FUNCTION__), __LINE__, *Color.ToString());
			}
		}
		return true;
	}
	return false;
}


// Add information about the semantic color (return true if all the fields were filled)
bool USLVisMaskVisualizer::AddSemanticColorInfo(FColor Color, const FString& ColorHex, UObject* Owner)
{
	FSLVisSemanticColorInfo SemColInfo;
	SemColInfo.Owner = Owner;
	SemColInfo.ColorHex = ColorHex;
	SemColInfo.Color = Color;
	SemColInfo.Class = FTags::GetValue(Owner, "SemLog", "Class");
	SemColInfo.Id = FTags::GetValue(Owner, "SemLog", "Id");

	SemanticColorsInfo.Emplace(Color, SemColInfo);

	return SemColInfo.IsComplete();
}

