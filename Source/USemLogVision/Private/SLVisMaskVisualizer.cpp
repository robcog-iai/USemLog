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
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
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
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
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
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
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
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
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
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Unique color in img=%s|%s"), TEXT(__FUNCTION__), __LINE__, 
					*Color.ToString(), *Color.ToHex());
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

		for (const auto& Color : InBitmap)
		{
			FColor MutableColor = Color;
			UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Search and swithcing MutableColor=%s"),
				TEXT(__FUNCTION__), __LINE__, *MutableColor.ToString());
			if (USLVisMaskVisualizer::SearchAndSwitchWithSemanticColor(MutableColor))
			{
				UE_LOG(LogTemp, Error, TEXT("\t\t\t\t%s::%d MutableColor is now=%s"), TEXT(__FUNCTION__), __LINE__, 
					*MutableColor.ToString());
				continue;
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

	// TODO check if redundancy should be kept
	SemanticColorsInfo.Emplace(Color, SemColInfo);
	SemanticColors.Emplace(Color);


	UE_LOG(LogTemp, Warning, TEXT("%s::%d SC=%s"), TEXT(__FUNCTION__), __LINE__, *SemColInfo.ToString());

	FLinearColor FromPow = FLinearColor::FromPow22Color(Color);
	FColor FromPowF = FromPow.ToFColor(false);
	FColor FromPowSRGB = FromPow.ToFColor(true);

	UE_LOG(LogTemp, Warning, TEXT("\t FromPow22: \n\t\t FColorLin=%s|%s \n\t\t FColorSRGB=%s|%s"),		
		*FromPowF.ToString(),
		*FromPowF.ToHex(),
		*FromPowSRGB.ToString(),
		*FromPowSRGB.ToHex());

	FLinearColor FromSRGB = FLinearColor::FromSRGBColor(Color);
	FColor FromSRGBF = FromSRGB.ToFColor(false);
	FColor FromSRGBSRGB = FromSRGB.ToFColor(true);

	UE_LOG(LogTemp, Warning, TEXT("\t FromSRGB: \n\t\t FColorLin=%s|%s \n\t\t FColorSRGB=%s|%s"),
		*FromSRGBF.ToString(),
		*FromSRGBF.ToHex(),
		*FromSRGBSRGB.ToString(),
		*FromSRGBSRGB.ToHex());


	return SemColInfo.IsComplete();
}

// Compare against the semantic colors, if found switch (update color info during), returns true if the color has been switched
bool USLVisMaskVisualizer::SearchAndSwitchWithSemanticColor(FColor& OutColor)
{
	// Lambda for the FindByPredicate function, checks if the two colors are similar 
	auto FindPredicateLambda = [this, &OutColor](const FColor& SemColor)
	{
		return this->CompareWithTolerance(OutColor, SemColor);
	};

	// If the two colors are similar, apply the semantic value to it
	if (FColor* FoundSemanticColor = SemanticColors.FindByPredicate(FindPredicateLambda))
	{
		// Do the switch
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Before the switch %s --> %s"),
			TEXT(__FUNCTION__), __LINE__, *OutColor.ToString(), *FoundSemanticColor->ToString());
			OutColor = *FoundSemanticColor;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Adfter the switch %s --> %s"), TEXT(__FUNCTION__), __LINE__,
			*OutColor.ToString(), *FoundSemanticColor->ToString());
			return true;
	}
	return false;
}

