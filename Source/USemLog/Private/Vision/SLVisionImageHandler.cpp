// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionImageHandler.h"
#include "SLEntitiesManager.h"

// Ctor
FSLVisionImageHandler::FSLVisionImageHandler()
{
	bIsInit = false;
}

// Load the color to entities mapping
bool FSLVisionImageHandler::Init()
{
	if(!bIsInit)
	{
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Init failed, init entities manager first.."), *FString(__func__), __LINE__);
			return false;
		}

		// Setup NON-skeletal entity mapping
		for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
		{
			if (Pair.Value.HasVisualMask())
			{
				const FColor MaskColor = FColor::FromHex(Pair.Value.VisualMask);
				ColorToEntityInfo.Emplace(MaskColor, FSLVisionEntityInfo(Pair.Value.Class, Pair.Value.Id));
				OriginalMaskColors.Emplace(MaskColor);
			}
		}

		// Setup skeletal entity mapping
		for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSkeletalSemanticData())
		{
			const FString SkelClass = Pair.Value->OwnerSemanticData.Class;
			const FString SkelId = Pair.Value->OwnerSemanticData.Id;

			for (const auto& BonePair : Pair.Value->SemanticBonesData)
			{
				if (!BonePair.Value.VisualMask.IsEmpty())
				{
					const FColor MaskColor = FColor::FromHex(BonePair.Value.VisualMask);
					ColorToSkelInfo.Emplace(MaskColor, FSLVisionSkelInfo(SkelClass, SkelId, BonePair.Value.Class));
					OriginalMaskColors.Emplace(MaskColor);
				}
			}
		}

		if (ColorToEntityInfo.Num() == 0 && ColorToSkelInfo.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Init failed, no entities found.."), *FString(__func__), __LINE__);
			return false;
		}

		// DEBUG
		for(const auto& Pair : ColorToEntityInfo)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Entity Color: %s -> %s"),
				*FString(__func__), __LINE__, *Pair.Key.ToString(), *Pair.Value.Class);
		}
		for (const auto& Pair : ColorToSkelInfo)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skel Color: %s -> %s"),
				*FString(__func__), __LINE__, *Pair.Key.ToString(), *Pair.Value.BoneClass);
		}


		bIsInit = true;
		return true;
	}
	return true;
}

// Clear init flag and mappings
void FSLVisionImageHandler::Reset()
{
	bIsInit = false;
	ColorToEntityInfo.Empty();
	ColorToSkelInfo.Empty();
}

// Get entities from mask image
void FSLVisionImageHandler::GetEntities(const TArray<FColor>& InMaskBitmap, int32 ImgWidth, int32 ImgHeight, FSLVisionViewData& OutViewData) const
{
	// Index position of the image matrix in rows and columns (used for storing the bounding box of the entities)
	int32 RowIdx = 0;
	int32 ColIdx = 0;

	// Mapping of the color to the number of concurrences and its bounding box in the image
	TMap<FColor, FSLVisionImagePixelColorInfo> ColorsData;

	// Iterate pixels in image
	for(const auto& PixelColor : InMaskBitmap)
	{
		// Ignore the color black, (the margins could be black due to the aspect ratio)
		if(PixelColor != FColor::Black)
		{
			// Store the number of each pixel value and update its bounding box
			if (FSLVisionImagePixelColorInfo* ColorData = ColorsData.Find(PixelColor))
			{
				(*ColorData).Num++;

				if (RowIdx < (*ColorData).MinBB.Y)
				{
					(*ColorData).MinBB.Y = RowIdx;
				}
				if (RowIdx > (*ColorData).MaxBB.Y)
				{
					(*ColorData).MaxBB.Y = RowIdx;
				}
				if (ColIdx < (*ColorData).MinBB.X)
				{
					(*ColorData).MinBB.X = ColIdx;
				}
				if (ColIdx > (*ColorData).MaxBB.X)
				{
					(*ColorData).MaxBB.X = ColIdx;
				}
			}
			else
			{
				ColorsData.Emplace(PixelColor, FSLVisionImagePixelColorInfo(1, 
					FIntPoint(ImgWidth, ImgHeight), FIntPoint(0, 0)));
			}
		}

		// Update current pixel index position
		ColIdx++;

		// Check for row change
		if (ColIdx > ImgWidth - 1)
		{
			ColIdx = 0;
			RowIdx++;
		}
	}

	for(const auto& Pair : ColorsData)
	{
		// Check if the color belongs to a NON-skeletal entity
		if(const FSLVisionEntityInfo* EntityInfo = ColorToEntityInfo.Find(Pair.Key))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Color: %s -> %s : %s -> %d"),
				*FString(__func__), __LINE__, *Pair.Key.ToString(), *EntityInfo->Class, *EntityInfo->Id, Pair.Value.Num);

			OutViewData.Entities.Emplace(FSLVisionViewEntityData(Pair.Key.ToHex(), FString::FromInt(Pair.Value.Num)));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d NO SEMANTICS %s : %d"), *FString(__func__), __LINE__, *Pair.Key.ToString(), Pair.Value.Num);
		}

		// Check if the color belongs to a skeletal entity
	}
}

// Restore image (the screenshot image pixel colors are a bit offseted from the supposed mask value) and get the entities from mask image
void FSLVisionImageHandler::RestoreImageAndGetEntities(TArray<FColor>& MaskBitmap, int32 ImgWidth, int32 ImgHeight,
	FSLVisionViewData& OutViewData) const
{
	// Used to calculate the percentage of an entity in the image
	const int64 ImgTotalPixels = ImgWidth * ImgHeight;

	// Index position of the image matrix in rows and columns (used for storing the bounding box of the entities)
	int32 RowIdx = 0;
	int32 ColIdx = 0;

	// Mapping of the color to the number of concurrences and its bounding box in the image
	TMap<FColor, FSLVisionImagePixelColorInfo> ColorsData;

	// Iterate pixels in image
	for (auto& PixelColor : MaskBitmap)
	{
		//// TODO temp rm
		//FColor TempPrevCol = PixelColor;
		// Ignore the color black, (the margins could be black due to the aspect ratio)
		if (PixelColor != FColor::Black)
		{
			// Restore the pixel color to its original mask value
			RestoreColorValue(PixelColor);
			//if(RestoreColorValue(PixelColor))
			//{
			//	// TODO rm
			//	//UE_LOG(LogTemp, Error, TEXT("%s::%d Restored %s -> %s "), *FString(__func__), __LINE__, *TempPrevCol.ToString(), *PixelColor.ToString());
			//}

			// Store the number of each pixel value and update its bounding box
			if (FSLVisionImagePixelColorInfo* ColorData = ColorsData.Find(PixelColor))
			{
				(*ColorData).Num++;

				if (RowIdx < (*ColorData).MinBB.Y)
				{
					(*ColorData).MinBB.Y = RowIdx;
				}
				if (RowIdx > (*ColorData).MaxBB.Y)
				{
					(*ColorData).MaxBB.Y = RowIdx;
				}
				if (ColIdx < (*ColorData).MinBB.X)
				{
					(*ColorData).MinBB.X = ColIdx;
				}
				if (ColIdx > (*ColorData).MaxBB.X)
				{
					(*ColorData).MaxBB.X = ColIdx;
				}
			}
			else
			{
				ColorsData.Emplace(PixelColor, FSLVisionImagePixelColorInfo(1,
					FIntPoint(ImgWidth, ImgHeight), FIntPoint(0, 0)));
			}
		}

		// Update current pixel index position
		ColIdx++;

		// Check for row change
		if (ColIdx > ImgWidth - 1)
		{
			ColIdx = 0;
			RowIdx++;
		}
	}

	// Skeletal data needs to be filled with all the bones and then updated in the outgoing ViewData
	TMap<FString, FSLVisionViewSkelData> IdToSkelData;

	// Iterate colors data and get their data
	for (const auto& Pair : ColorsData)
	{
		// Check semantically annotated entity that belongs to the mask color
		if (const FSLVisionEntityInfo* EntityInfo = ColorToEntityInfo.Find(Pair.Key))
		{
			FSLVisionViewEntityData EntityData(EntityInfo->Id, EntityInfo->Class, Pair.Value.MinBB, Pair.Value.MaxBB);
			EntityData.ImagePercentage = (float) Pair.Value.Num / ImgTotalPixels;
			OutViewData.Entities.Emplace(EntityData);


			//UE_LOG(LogTemp, Warning, TEXT("%s::%d Color: [%s] -> %s : %s -> %d -> %f"),
			//	*FString(__func__), __LINE__,
			//	*Pair.Key.ToString(),
			//	*EntityData.Class,
			//	*EntityData.Id,
			//	Pair.Value.Num,
			//	EntityData.ImagePercentage);
		}
		else if(const FSLVisionSkelInfo* SkelInfo = ColorToSkelInfo.Find(Pair.Key))
		{
			FSLVisionViewSkelBoneData BoneData(SkelInfo->BoneClass, Pair.Value.MinBB, Pair.Value.MaxBB);
			BoneData.ImagePercentage = (float) Pair.Value.Num / ImgTotalPixels;

			// Update existing or create a new skeletal data
			if(FSLVisionViewSkelData* SkelData = IdToSkelData.Find(SkelInfo->Id))
			{
				SkelData->Bones.Emplace(BoneData);
			}
			else
			{
				FSLVisionViewSkelData NewSkelData(SkelInfo->Id, SkelInfo->Class);
				NewSkelData.Bones.Emplace(BoneData);
				IdToSkelData.Emplace(SkelInfo->Id, NewSkelData);
			}


			//UE_LOG(LogTemp, Warning, TEXT("%s::%d Color: %s -> %s : %s : %s -> %d"),
			//	*FString(__func__), __LINE__, *Pair.Key.ToString(),
			//	*SkelInfo->Class, *SkelInfo->Id, *SkelInfo->BoneClass, Pair.Value.Num);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d NO ENTITY SEMANTICS %s : %d"), *FString(__func__), __LINE__, *Pair.Key.ToString(), Pair.Value.Num);
		}
	}

	// Add the skeletal data
	for(auto& Pair : IdToSkelData)
	{
		// Calculate bones related parameters of the whole skeletal entity
		Pair.Value.CalculateParamsFromBones();
		OutViewData.SkelEntities.Emplace(Pair.Value);
	}
}

// Restore the color of the pixel to its original mask value (offseted by screenshot rendering artifacts), returns true if restoration happened
FORCEINLINE bool FSLVisionImageHandler::RestoreColorValue(FColor& PixelColor) const
{
	// Check if the PixelColor is almost equal to any of the stored original mask color values
	if(const FColor* OrigColor = OriginalMaskColors.FindByPredicate(
		[&PixelColor](const FColor& Item) { return AlmostEqual(PixelColor, Item, ColorTolerance);} ))
	{
		PixelColor = *OrigColor;
		return true;
	}
	return false;
}
