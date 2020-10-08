// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionMaskImageHandler.h"


// Ctor
FSLVisionMaskImageHandler::FSLVisionMaskImageHandler()
{
	bIsInit = false;
}

// Load the color to entities mapping
bool FSLVisionMaskImageHandler::Init()
{
	if(!bIsInit)
	{
		//// Setup NON-skeletal entity mapping
		//for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSemanticData())
		//{
		//	if (Pair.Value.HasVisualMask() && Pair.Value.HasRenderedVisualMask())
		//	{
		//		const FColor RenderedMaskColor = FColor::FromHex(Pair.Value.RenderedVisualMask);
		//		RenderedColorToEntityInfo.Emplace(RenderedMaskColor, 
		//			FSLVisionMaskEntityInfo(Pair.Value.Class, Pair.Value.Id, Pair.Value.VisualMask));
		//	}
		//	else
		//	{
		//		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Pair.Key))
		//		{
		//			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no visual or rendered (calibrated) visual mask.."),
		//				*FString(__func__), __LINE__, *SMA->GetName());
		//		}
		//	}
		//}

		//// Setup skeletal entity mapping
		//for (const auto& Pair : FSLEntitiesManager::GetInstance()->GetObjectsSkeletalSemanticData())
		//{
		//	const FString SkelClass = Pair.Value->OwnerSemanticData.Class;
		//	const FString SkelId = Pair.Value->OwnerSemanticData.Id;

		//	for (const auto& BonePair : Pair.Value->SemanticBonesData)
		//	{
		//		if (BonePair.Value.HasVisualMask() && BonePair.Value.HasCalibratedVisualMask())
		//		{
		//			const FColor RenderedMaskColor = FColor::FromHex(BonePair.Value.CalibratedVisualMask);
		//			RenderedColorToSkelInfo.Emplace(RenderedMaskColor, 
		//				FSLVisionMaskSkelInfo(SkelClass, SkelId, BonePair.Value.Class, BonePair.Value.VisualMask));
		//		}
		//		else
		//		{
		//			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s - %s has no visual or rendered (calibrated) visual mask.."),
		//				*FString(__func__), __LINE__, *Pair.Key->GetName(), *BonePair.Key.ToString());
		//		}
		//	}
		//}

		if (RenderedColorToEntityInfo.Num() == 0 && RenderedColorToSkelInfo.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Init failed, no entities found.."), *FString(__func__), __LINE__);
			return false;
		}

		//// DEBUG
		//for(const auto& Pair : RenderedColorToEntityInfo)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Entity\t [%s:%s]:\t\t %s -> %s"),
		//		*FString(__func__), __LINE__, *Pair.Value.Class, *Pair.Value.Id, *Pair.Key.ToString(),
		//		*FColor::FromHex(Pair.Value.OrigMaskColor).ToString());
		//}
		//for (const auto& Pair : RenderedColorToSkelInfo)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Skel\t [%s:%s:%s]:\t\t %s -> %s"),
		//		*FString(__func__), __LINE__, *Pair.Value.Class, *Pair.Value.Id, *Pair.Value.BoneClass, *Pair.Key.ToString(),
		//		*FColor::FromHex(Pair.Value.OrigMaskColor).ToString());
		//}

		bIsInit = true;
		return true;
	}
	return true;
}

// Clear init flag and mappings
void FSLVisionMaskImageHandler::Reset()
{
	bIsInit = false;
	RenderedColorToEntityInfo.Empty();
	RenderedColorToSkelInfo.Empty();
}

// Restore image (the screenshot image pixel colors are a bit offseted from the supposed mask value) and get the entities from mask image
void FSLVisionMaskImageHandler::GetDataAndRestoreImage(TArray<FColor>& MaskBitmapToRestore, int32 ImgWidth, int32 ImgHeight,
	FSLVisionViewData& OutViewData) const
{
	// Used to calculate the percentage of an entity in the image
	const int64 ImgTotalPixels = ImgWidth * ImgHeight;

	// Index position of the image matrix in rows and columns (used for storing the bounding box of the entities)
	int32 RowIdx = 0;
	int32 ColIdx = 0;

	// Store the occurance of every color in the image to its data
	TMap<FColor, FSLVisionImageColorInfo> TempRenderedColorsData;

	// Array of rendered colors without a semantic match (store in array to avoid smapping the logger everytime the color appears)
	TSet<FColor> UnknownColors;

	// Restore image colors and create a mapping of all the rendered pixel colors to its data
	for (auto& PixelColor : MaskBitmapToRestore)
	{
		// Ignore color black (represents semantically unknown areas, normally there should not be any
		if (PixelColor != FColor::Black)
		{
			// Check if the color is new
			if (FSLVisionImageColorInfo* ColorData = TempRenderedColorsData.Find(PixelColor))
			{
				// Color already stored, update its data
				(*ColorData).Num++;

				// Update the bounding box in the image
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

				// Fix image by changing the rendered color to the original value
				PixelColor = (*ColorData).OriginalMaskColor;
			}
			else
			{
				// Cache rendered color, since the pixel color will be restored to the original mask (if found)
				const FColor RenderedColor = PixelColor;

				// New color, cache its info
				FSLVisionImageColorInfo ColorInfo(1, FIntPoint(ImgWidth, ImgHeight), FIntPoint(0, 0));

				// Get the original mask color value
				if (const FSLVisionMaskEntityInfo* EntityInfo = RenderedColorToEntityInfo.Find(RenderedColor))
				{
					// Color found as an entity visual mask
					ColorInfo.OriginalMaskColor = FColor::FromHex((*EntityInfo).OrigMaskColor);

					// Cache the color info in the local map
					TempRenderedColorsData.Emplace(RenderedColor, ColorInfo);

					// Fix image by changing the rendered color to the original value
					PixelColor = ColorInfo.OriginalMaskColor;
				}
				else if (const FSLVisionMaskSkelInfo* SkelInfo = RenderedColorToSkelInfo.Find(RenderedColor))
				{
					// Color found as a skeletal visual mask
					ColorInfo.OriginalMaskColor = FColor::FromHex((*SkelInfo).OrigMaskColor);

					// Cache the color info in the local map
					TempRenderedColorsData.Emplace(RenderedColor, ColorInfo);

					// Fix image by changing the rendered color to the original value
					PixelColor = ColorInfo.OriginalMaskColor;
				}
				else if(!UnknownColors.Contains(PixelColor))
				{
					UnknownColors.Add(PixelColor);
					UE_LOG(LogTemp, Error, TEXT("%s::%d Rendered color %s - %s has no mapping to any entity.. this should not happen.."),
						*FString(__func__), __LINE__, *RenderedColor.ToString(), *RenderedColor.ToHex());
				}
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

	// Store skeletal related data in a temp map, this will need an extra processing to calculcate the data as a whole skeleton (from bones)
	TMap<FString, FSLVisionViewSkelData> TempIdToSkelData;

	// Iterate the collected data from the image
	for (const auto& Pair : TempRenderedColorsData)
	{
		const FColor RenderedColor = Pair.Key;
		
		// Check semantically annotated entity that belongs to the mask color
		if (const FSLVisionMaskEntityInfo* EntityInfo = RenderedColorToEntityInfo.Find(RenderedColor))
		{
			FSLVisionViewEntityData EntityData(EntityInfo->Id, EntityInfo->Class, Pair.Value.MinBB, Pair.Value.MaxBB);
			EntityData.ImagePercentage = (float) Pair.Value.Num / ImgTotalPixels;
			OutViewData.Entities.Emplace(EntityData);

			//// DEBUG
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d Color: [%s] -> %s : %s -> %d -> %f"),
			//	*FString(__func__), __LINE__,
			//	*RenderedColor.ToString(),
			//	*EntityData.Class,
			//	*EntityData.Id,
			//	Pair.Value.Num,
			//	EntityData.ImagePercentage);
		}
		else if(const FSLVisionMaskSkelInfo* SkelInfo = RenderedColorToSkelInfo.Find(RenderedColor))
		{
			// Collect bone data
			FSLVisionViewSkelBoneData BoneData(SkelInfo->BoneClass, Pair.Value.MinBB, Pair.Value.MaxBB);
			BoneData.ImagePercentage = (float) Pair.Value.Num / ImgTotalPixels;

			// Update existing or create a new skeletal data
			if(FSLVisionViewSkelData* SkelData = TempIdToSkelData.Find(SkelInfo->Id))
			{
				SkelData->Bones.Emplace(BoneData);
			}
			else
			{
				FSLVisionViewSkelData NewSkelData(SkelInfo->Id, SkelInfo->Class);
				NewSkelData.Bones.Emplace(BoneData);
				TempIdToSkelData.Emplace(SkelInfo->Id, NewSkelData);
			}

			// DEBUG
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d Color: %s -> %s : %s : %s -> %d"),
			//	*FString(__func__), __LINE__, *Pair.Key.ToString(),
			//	*SkelInfo->Class, *SkelInfo->Id, *SkelInfo->BoneClass, Pair.Value.Num);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Rendered color %s - %s has no mapping to any entity.. this should not happen.."),
				*FString(__func__), __LINE__, *RenderedColor.ToString(), *RenderedColor.ToHex());
		}
	}

	// Add the skeletal data from the bones
	for(auto& Pair : TempIdToSkelData)
	{
		// Calculate bones related parameters of the whole skeletal entity
		Pair.Value.CalculateParamsFromBones();
		OutViewData.SkelEntities.Emplace(Pair.Value);
	}
}

// Restore the color of the pixel to its original mask value (offseted by screenshot rendering artifacts), returns true if restoration happened
bool FSLVisionMaskImageHandler::RestoreColorValueFromArray(FColor& RenderedPixelColor, const TArray<FColor>& InOriginalMaskColors, uint8 Tolerance) const
{
	// Check if the PixelColor is almost equal to any of the stored original mask color values
	if(const FColor* OrigColor = InOriginalMaskColors.FindByPredicate(
		[&RenderedPixelColor, &Tolerance](const FColor& Item) { return AlmostEqual(RenderedPixelColor, Item, Tolerance);} ))
	{
		RenderedPixelColor = *OrigColor;
		return true;
	}
	return false;
}
