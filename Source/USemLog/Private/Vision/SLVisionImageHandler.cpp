// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionImageHandler.h"
#include "SLEntitiesManager.h"

// Ctor
FSLVisionImageHandler::FSLVisionImageHandler()
{
}

// Load the color to entities mapping
void FSLVisionImageHandler::Init()
{
	for(const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
	{
		if(Pair.Value.HasVisualMask())
		{
			ColorToEntityData.Emplace(FColor::FromHex(Pair.Value.VisualMask),
				FSLVisionEntityClassAndId(Pair.Value.Class, Pair.Value.Id));
		}
	}

	for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSkeletalSemanticData())
	{
		const FString SkelClass = Pair.Value->OwnerSemanticData.Class;
		const FString SkelId = Pair.Value->OwnerSemanticData.Id;

		for(const auto& BonePair : Pair.Value->SemanticBonesData)
		{
			if(!BonePair.Value.VisualMask.IsEmpty())
			{
				ColorToSKelEntityData.Emplace(FColor::FromHex(BonePair.Value.VisualMask),
					FSLVisionSkelEntityClassAndId(SkelClass, SkelId, BonePair.Value.Class));
			}
		}
	}
}

// Get entities from mask image
void FSLVisionImageHandler::GetEntities(const TArray<FColor>& InMaskBitmap, int32 ImgWidth, int32 ImgHeight, FSLVisionViewData& OutViewData) const
{
	// Index position of the image matrix in rows and columns (used for storing the bounding box of the entities)
	int32 RowIdx = 0;
	int32 ColIdx = 0;

	// Mapping of the color to the number of concurrences and its bounding box in the image
	TMap<FColor, FSLVisionImageColorData> ColorsData;

	// Iterate pixels in image
	for(const auto& PixelColor : InMaskBitmap)
	{
		// Ignore the color black, (the margins could be black due to the aspect ratio)
		if(PixelColor != FColor::Black)
		{
			// Store the number of each pixel value and update its bounding box
			if (FSLVisionImageColorData* ColorData = ColorsData.Find(PixelColor))
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
				ColorsData.Emplace(PixelColor, FSLVisionImageColorData(0, 
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s : %d"), *FString(__func__), __LINE__, *Pair.Key.ToString(), Pair.Value.Num);
		OutViewData.Entities.Emplace(FSLVisionViewEntitiyData(Pair.Key.ToHex(), FString::FromInt(Pair.Value.Num)));
	}
}
