// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisMaskHandler.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "Tags.h"

// Ctor
USLVisMaskHandler::USLVisMaskHandler()
{
	bIsInit = false;
	bMaskMaterialsOn = false;
};

// Dtor
USLVisMaskHandler::~USLVisMaskHandler()
{
}

// Init
void USLVisMaskHandler::Init()
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
					USLVisMaskHandler::AddSemanticData(SemColor, ColorHex, SMAItr->Tags);
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
						USLVisMaskHandler::AddSemanticData(SemColor, ColorHex, SMC->ComponentTags);
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
					USLVisMaskHandler::AddSemanticData(SemColor, ColorHex, SkMAItr->Tags);
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
						USLVisMaskHandler::AddSemanticData(SemColor, ColorHex, SkMC->ComponentTags);
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
bool USLVisMaskHandler::ApplyMaskMaterials()
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
bool USLVisMaskHandler::ApplyOriginalMaterials()
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
bool USLVisMaskHandler::ToggleMaterials()
{
	if (bMaskMaterialsOn)
	{
		return USLVisMaskHandler::ApplyOriginalMaterials();
	}
	else
	{
		return USLVisMaskHandler::ApplyMaskMaterials();
	}
}

// Process the semantic mask image, fix pixel color deviations in image, return entities data
void USLVisMaskHandler::ProcessMaskImage(TArray<FColor>& MaskImage, TArray<FSLVisEntitiyData>& OutEntitiesData)
{
	// Map for easy updating of the entity data and avoiding duplicates
	TMap<FColor, FSLVisEntitiyData> ColorToEntityData;
	TSet<FColor> ColorsInRange;
	TSet<FColor> ColorsOutOfRange;

	// Image array index value
	int32 Idx = 0;

	// Iterate mask image
	for (auto& Color : MaskImage)
	{
		// Continue if it is different than black with a tolerance
		if (!USLVisMaskHandler::AlmostEqual(Color, FColor::Black, 9))
		{
			FColor TempColor = Color;
			// Check and replace if color got deviated from the semantic one due to conversions (FLinearColor to FColor)
			if (USLVisMaskHandler::RestoreIfAlmostSemantic(Color))
			{
				ColorsInRange.Add(TempColor);
			}
			else

			// Check if color has a semantic meaning
			if (SemanticColorData.Contains(Color))
			{
				// Check if color has been cached in the temp map
				if(ColorToEntityData.Contains(Color))
				{
					// Update the existing data
					ColorToEntityData[Color].NumPixels++;
					ColorToEntityData[Color].Indexes.Add(Idx);
				}
				else
				{
					// Add init entity data
					ColorToEntityData.Emplace(Color, SemanticColorData[Color]);
				}
			}
			else
			{
				ColorsOutOfRange.Add(Color);
				Color = FColor::Black;
			}
		}

		// Increment array position index 
		Idx++;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic colors:"), TEXT(__FUNCTION__), __LINE__);
	for (const auto& C : SemanticColors)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *C.ToString());
	}

	UE_LOG(LogTemp, Warning, TEXT("%s::%d Colors in range:"), TEXT(__FUNCTION__), __LINE__);
	for (const auto& C : ColorsInRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *C.ToString());
	}

	UE_LOG(LogTemp, Error, TEXT("%s::%d Colors OUT OF range:"), TEXT(__FUNCTION__), __LINE__);
	for (const auto& C : ColorsOutOfRange)
	{
		UE_LOG(LogTemp, Error, TEXT("\t\t%s"), *C.ToString());
	}

	// Set output data
	ColorToEntityData.GenerateValueArray(OutEntitiesData);
}

// Add information about the semantic color (return true if all the fields were filled)
void USLVisMaskHandler::AddSemanticData(const FColor& Color, const FString& ColorHex, const TArray<FName>& Tags)
{
	if (USLVisMaskHandler::AlmostEqual(Color, FColor::Black, 5))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Semantic color is (almost) black, this should not happen, skipping.."), TEXT(__FUNCTION__), __LINE__);
		return;
	}
	FSLVisEntitiyData EntityData;
	EntityData.Color = Color;
	EntityData.ColorHex = ColorHex;
	EntityData.Class = FTags::GetValue(Tags, "SemLog", "Class");
	EntityData.Id = FTags::GetValue(Tags, "SemLog", "Id");

	SemanticColors.Emplace(Color);
	SemanticColorData.Emplace(Color, EntityData);

	UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityData=\n\t%s"), TEXT(__FUNCTION__), __LINE__, *EntityData.ToString());
}

// Compare against the semantic colors, if found switch (update color info during), returns true if the color has been switched
bool USLVisMaskHandler::RestoreIfAlmostSemantic(FColor& OutColor)
{
	// Lambda for the FindByPredicate function, checks if the two colors are similar 
	auto AlmostEqualPredicate = [this, &OutColor](const FColor& SemColor)
	{
		return this->AlmostEqual(OutColor, SemColor, 21);
	};

	// If the two colors are similar, replace it with the semantic value
	if (FColor* FoundSemanticColor = SemanticColors.FindByPredicate(AlmostEqualPredicate))
	{
		// Do the switch
		OutColor = *FoundSemanticColor;
		return true;
	}
	return false;
}

