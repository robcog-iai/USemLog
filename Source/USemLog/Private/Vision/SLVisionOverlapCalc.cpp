// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionOverlapCalc.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"

#include "Vision/SLVisionStructs.h"
//#include "Skeletal/SLSkeletalDataComponent.h"
#include "SLVisionLogger.h"


// Constructor
USLVisionOverlapCalc::USLVisionOverlapCalc() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	EntityIndex = INDEX_NONE;
	SkelIndex = INDEX_NONE;
	BoneIndex = INDEX_NONE;
	CurrBoneMaterialIndex = INDEX_NONE;
	CurrOverlapCalcIdx = INDEX_NONE;
	CurrSMAClone = nullptr;
	CurrPMAClone = nullptr;
	bSkelArrayActive = false;
	bSkelBoneActive = false;
}

// Destructor
USLVisionOverlapCalc::~USLVisionOverlapCalc()
{
}

// Give control to the overlap calc to pause and start its parent (vision logger)
void USLVisionOverlapCalc::Init(USLVisionLogger* InParent, FIntPoint InResolution, const FString& InSaveLocallyPath)
{
	if (!bIsInit)
	{
		CurrOverlapCalcIdx = 0;
		Parent = InParent;
		ViewportClient = GetWorld()->GetGameViewport();
		Resolution = InResolution;
		SaveLocallyFolderName = InSaveLocallyPath;

		// Load the default mask material
		// this will be used as a template to create the non-occluding mask materials to add to the clones
		DefaultNonOccludingMaterial = LoadObject<UMaterial>(this,
			TEXT("/USemLog/CV/M_SLNonOccludingDefaultMask.M_SLNonOccludingDefaultMask"));
		if (!DefaultNonOccludingMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."), *FString(__func__), __LINE__);
			return;
		}
		DefaultNonOccludingMaterial->bUsedWithStaticLighting = true;
		DefaultNonOccludingMaterial->bUsedWithSkeletalMesh = true;
		DefaultNonOccludingMaterial->bDisableDepthTest = true;

		if (Parent && ViewportClient)
		{
			bIsInit = true;
		}
	}
}

// Calculate overlaps for the given scene
void USLVisionOverlapCalc::Start(FSLVisionViewData* CurrViewData, float Timestamp, int32 FrameIdx)
{
	if (!bIsStarted && bIsInit)
	{
		CurrTs = Timestamp;
		CurrFrameIdx = FrameIdx;
		SubFolderName.Empty();
		SubFolderName = CurrViewData->Class + "_O";
		Entities = &CurrViewData->Entities;
		SkelEntities = &CurrViewData->SkelEntities;

		CurrOverlapCalcIdx = 0;		
		int32 NumBones = 0;
		for (const auto& SkE : *SkelEntities)
		{
			NumBones += SkE.Bones.Num();
		}
		TotalOverlapCalcNum = Entities->Num() + SkelEntities->Num() + NumBones;
		
		if (!SelectFirstItem())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No items found in the scene.."), *FString(__func__), __LINE__);
			return;
		}

		ApplyNonOccludingMaterial();


		// Switch callback functions and pause parent
		Parent->Pause(true);
		ScreenshotCallbackHandle = ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLVisionOverlapCalc::ScreenshotCB);

		// Overwrite to the overlap calc screenshot resolution
		InitScreenshotResolution(Resolution);

		RequestScreenshot();
		
		bIsFinished = false;
		bIsStarted = true;		
	}
}

// Reset all flags and temporaries, called when the scene overlaps are calculated, this un-pauses the parent as well
void USLVisionOverlapCalc::Finish()
{
	if (!bIsFinished && bIsStarted)
	{
		CurrOverlapCalcIdx = INDEX_NONE;
		EntityIndex = INDEX_NONE;
		SkelIndex = INDEX_NONE;
		
		bSkelArrayActive = false;
		
		Entities = nullptr;
		SkelEntities = nullptr;

		CurrSMAClone = nullptr;
		CurrPMAClone = nullptr;

		// Switch callback functions, and re-start parent
		ViewportClient->OnScreenshotCaptured().Remove(ScreenshotCallbackHandle);
		Parent->Pause(false);

		// Mark logger as finished
		bIsStarted = false;
		bIsFinished = true;
	}
}

// Trigger the screenshot on the game thread
void USLVisionOverlapCalc::RequestScreenshot()
{
	// Sec-Ms_FrameNum_OverlapFrame_Viewmode
	CurrImageFilename = FString::Printf(TEXT("%.2f"), CurrTs).Replace(TEXT("."), TEXT("-")) + "_" +
		FString::FromInt(CurrFrameIdx) + "_" + FString::FromInt(CurrOverlapCalcIdx) + "_O";

	GetHighResScreenshotConfig().FilenameOverride = CurrImageFilename;

	AsyncTask(ENamedThreads::GameThread, [this]()
	{		
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when the screenshot is captured
void USLVisionOverlapCalc::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	// Terminal output with the log progress
	PrintProgress();

	// Calcuate overlap for the currently selected item
	CalculateOverlap(Bitmap, SizeX, SizeY);

	// Save the png locally
	if (!SaveLocallyFolderName.IsEmpty())
	{
		// Compress image
		TArray<uint8> CompressedBitmap;
		FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);
		FString Path = FPaths::ProjectDir() + "/SemLog/" + SaveLocallyFolderName + "/" + SubFolderName + "/" + CurrImageFilename + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	// Re-apply original material before selecting the next item
	ReApplyOriginalMaterial();

	// Check if there are any other items in the scene
	if (SelectNextItem())
	{
		CurrOverlapCalcIdx++;
		ApplyNonOccludingMaterial();
		RequestScreenshot();
	}
	else
	{
		// Finish overlap calculation and conitnue with the vision logger
		Finish();
	}	
}

// Init hi-res screenshot resolution
void USLVisionOverlapCalc::InitScreenshotResolution(FIntPoint InResolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(InResolution.X, InResolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Select the first item (static or skeletal)
bool USLVisionOverlapCalc::SelectFirstItem()
{
	if (SelectFirstEntity())
	{
		return true;
	}
	else if (SelectFirstSkel())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Select the next item (static or skeletal), return false when no more items are available
bool USLVisionOverlapCalc::SelectNextItem()
{
	if (!bSkelArrayActive)
	{
		if (SelectNextEntity())
		{
			return true;
		}
		else
		{
			return SelectFirstSkel();
		}
	}
	else
	{
		if (!bSkelBoneActive)
		{
			return SelectFirstSkelBone();
		}
		else
		{
			if(SelectNextSkelBone())
			{
				return true;
			}
			else
			{
				return SelectNextSkel();
			}
		}		
	}
}

// Select the first entity in the array (if not empty)
bool USLVisionOverlapCalc::SelectFirstEntity()
{
	if (EntityIndex == INDEX_NONE && Entities && Entities->Num() > 0)
	{
		EntityIndex = 0;
		CurrSMAClone = Parent->GetStaticMeshMaskCloneFromId((*Entities)[EntityIndex].Id);
		if (!CurrSMAClone)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to entity %s - %s, continuing.."),
				*FString(__func__), __LINE__, *(*Entities)[EntityIndex].Class, *(*Entities)[EntityIndex].Id);
			return SelectNextEntity();
		}
		return true;
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no entity array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Select the next entity in the array (if available)
bool USLVisionOverlapCalc::SelectNextEntity()
{
	if (EntityIndex != INDEX_NONE)
	{
		EntityIndex++;
		if (!Entities->IsValidIndex(EntityIndex))
		{
			EntityIndex = INDEX_NONE;
			CurrSMAClone = nullptr;
			return false;
		}
		else
		{
			CurrSMAClone = Parent->GetStaticMeshMaskCloneFromId((*Entities)[EntityIndex].Id);
			if (!CurrSMAClone)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to entity %s - %s, continuing.."),
					*FString(__func__), __LINE__, *(*Entities)[EntityIndex].Class, *(*Entities)[EntityIndex].Id);
				return SelectNextEntity();
			}
			return true;
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d First entity was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Select the first skel entity in the array (if not empty)
bool USLVisionOverlapCalc::SelectFirstSkel()
{
	if (SkelIndex == INDEX_NONE && SkelEntities && SkelEntities->Num() > 0)
	{		
		SkelIndex = 0;
		bSkelArrayActive = true;

		//CurrPMAClone = Parent->GetPoseableSkeletalMaskCloneFromId((*SkelEntities)[SkelIndex].Id, &CurrSkelDataComp);

		//if (!CurrPMAClone || !CurrSkelDataComp)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to skel entity or its data component %s - %s, continuing.."),
		//		*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
		//	return SelectNextSkel();
		//}
		return true;
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no skel array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Select the next skeletal entity in the array (if available)
bool USLVisionOverlapCalc::SelectNextSkel()
{
	if (SkelIndex != INDEX_NONE)
	{
		SkelIndex++;
		if (!SkelEntities->IsValidIndex(SkelIndex))
		{
			SkelIndex = INDEX_NONE;
			CurrPMAClone = nullptr;
			return false;
		}
		else
		{
			//CurrPMAClone = Parent->GetPoseableSkeletalMaskCloneFromId((*SkelEntities)[SkelIndex].Id, &CurrSkelDataComp);
			//if (!CurrPMAClone)
			//{
			//	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to skel entity %s - %s, continuing.."),
			//		*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
			//	return SelectNextSkel();
			//}
			return true;;
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d First skel was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Select the first skel bone in the array
bool USLVisionOverlapCalc::SelectFirstSkelBone()
{
	if (BoneIndex == INDEX_NONE && SkelIndex != INDEX_NONE && (*SkelEntities)[SkelIndex].Bones.Num() > 0)
	{
		BoneIndex = 0;
		bSkelBoneActive = true;
		CurrBoneMaterialIndex = GetMaterialIndexOfCurrentlySelectedBone();
		if (CurrBoneMaterialIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find bone index to skel entity %s - %s - %s, continuing.."),
				*FString(__func__), __LINE__, 
				*(*SkelEntities)[SkelIndex].Class,
				*(*SkelEntities)[SkelIndex].Id,
				*(*SkelEntities)[SkelIndex].Bones[BoneIndex].Class);
			return SelectNextSkelBone();
		}
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Select the next skeletal bone in the array (if available)
bool USLVisionOverlapCalc::SelectNextSkelBone()
{
	if (BoneIndex != INDEX_NONE)
	{
		BoneIndex++;
		if (!(*SkelEntities)[SkelIndex].Bones.IsValidIndex(BoneIndex))
		{
			BoneIndex = INDEX_NONE;
			CurrBoneMaterialIndex = INDEX_NONE;
			bSkelBoneActive = false;
			return false;
		}
		else
		{
			CurrBoneMaterialIndex = GetMaterialIndexOfCurrentlySelectedBone();
			if (CurrBoneMaterialIndex == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find bone index to skel entity %s - %s - %s, continuing.."),
					*FString(__func__), __LINE__,
					*(*SkelEntities)[SkelIndex].Class,
					*(*SkelEntities)[SkelIndex].Id,
					*(*SkelEntities)[SkelIndex].Bones[BoneIndex].Class);
				return SelectNextSkelBone();
			}
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Apply the non occluding material to the currently selected item
void USLVisionOverlapCalc::ApplyNonOccludingMaterial()
{
	// Create the non-occluding material
	UMaterialInstanceDynamic* NonOccludingDynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultNonOccludingMaterial, GetTransientPackage());
	NonOccludingDynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::White);

	if (!bSkelArrayActive && CurrSMAClone)
	{
		if (UStaticMeshComponent* MC = CurrSMAClone->GetStaticMeshComponent())
		{
			int32 TotalNumMaterials = MC->GetNumMaterials();
			if (TotalNumMaterials > 0)
			{
				// Set array length
				CachedMaterials.AddZeroed(TotalNumMaterials);
				for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
				{
					// Cache original material
					CachedMaterials[MaterialIndex] = MC->GetMaterial(MaterialIndex);

					// Switch to non occluding material
					MC->SetMaterial(MaterialIndex, NonOccludingDynamicMaskMaterial);
				}
			}			
		}
	}
	else if(CurrPMAClone)
	{	
		if (!bSkelBoneActive)
		{			
			if (UPoseableMeshComponent* PMC = CurrPMAClone->GetPoseableMeshComponent())
			{
				// Apply to whole skeleton
				int32 TotalNumMaterials = PMC->GetNumMaterials();
				if (TotalNumMaterials > 0)
				{
					// Set array length
					CachedMaterials.AddZeroed(TotalNumMaterials);
					for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
					{
						// Cache original material
						CachedMaterials[MaterialIndex] = PMC->GetMaterial(MaterialIndex);

						// Switch to non occluding material
						PMC->SetMaterial(MaterialIndex, NonOccludingDynamicMaskMaterial);
					}
				}
			}
		}
		else
		{
			// Cache only the active bone
			if (CurrBoneMaterialIndex != INDEX_NONE)
			{
				// Cache original material
				CachedMaterials.AddZeroed(1);
				CachedMaterials[0] = CurrPMAClone->GetPoseableMeshComponent()->GetMaterial(CurrBoneMaterialIndex);

				// Add non occluding material
				CurrPMAClone->SetCustomMaterial(CurrBoneMaterialIndex, NonOccludingDynamicMaskMaterial);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
	}
}

// Re-apply the original material to the currently selected item
void USLVisionOverlapCalc::ReApplyOriginalMaterial()
{
	if (!bSkelArrayActive && CurrSMAClone)
	{
		if (UStaticMeshComponent* MC = CurrSMAClone->GetStaticMeshComponent())
		{
			int32 TotalNumMaterials = MC->GetNumMaterials();
			if (TotalNumMaterials > 0)
			{
				for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
				{
					// Switch material
					MC->SetMaterial(MaterialIndex, CachedMaterials[MaterialIndex]);
				}
			}
			CachedMaterials.Empty();
		}		

	}
	else if(CurrPMAClone)
	{
		if (!bSkelBoneActive)
		{
			if (UPoseableMeshComponent* PMC = CurrPMAClone->GetPoseableMeshComponent())
			{
				int32 TotalNumMaterials = PMC->GetNumMaterials();
				if (TotalNumMaterials > 0)
				{
					for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
					{
						PMC->SetMaterial(MaterialIndex, CachedMaterials[MaterialIndex]);
					}
				}
				CachedMaterials.Empty();
			}
		}
		else
		{
			// Apply only to the active bone
			if (CurrBoneMaterialIndex != INDEX_NONE)
			{
				// Add original cached material
				CurrPMAClone->SetCustomMaterial(CurrBoneMaterialIndex, CachedMaterials[0]);
				CachedMaterials.Empty();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
	}
}

// Calculate overlap
void USLVisionOverlapCalc::CalculateOverlap(const TArray<FColor>& NonOccludedImage, int32 ImgWidth, int32 ImgHeight)
{
	// Index position of the image matrix in rows and columns (used to check if the entity is clipped)
	int32 RowIdx = 0;
	int32 ColIdx = 0;

	// Used to calculate the percentage of an entity in the image
	const int64 ImgTotalPixels = ImgWidth * ImgHeight;
	int64 NumWhitePixels = 0;
	bool bIsClipped = false;

	// Count the number of white pixels
	for (const auto& Pixel : NonOccludedImage)
	{
		if (Pixel == FColor::White)
		{
			NumWhitePixels++;

			if (ColIdx == 0 || ColIdx > ImgWidth - 1 || RowIdx == 0 || RowIdx > ImgHeight - 1)
			{
				bIsClipped = true;
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

	// Percentage of the image with white pixels (the non occluded object)
	float NonOccImgPerc = (float) NumWhitePixels / ImgTotalPixels;
	
	if (!bSkelArrayActive)
	{
		// Set percentage  (non occ image perc - occ image perc / non occ image perc)
		const float ImgPercDiff = (NonOccImgPerc - (*Entities)[EntityIndex].ImagePercentage);
		const float OccPerc = ImgPercDiff / NonOccImgPerc;
		(*Entities)[EntityIndex].OcclusionPercentage = OccPerc < 0.01f ? 0.f : OccPerc;

		// Set flag showing if the entity is clipped (touches the edge of the image)
		(*Entities)[EntityIndex].bIsClipped = bIsClipped;

		//// DEBUG
		//UE_LOG(LogTemp, Error, TEXT("%s::%d [%s-%s] \t\t ImgPerc=%.8f; NonOccImgPerc=%.8f; OccPerc=%.8f; bIsClipped=%d;"),
		//	*FString(__func__), __LINE__, *(*Entities)[EntityIndex].Class, *(*Entities)[EntityIndex].Id,
		//	(*Entities)[EntityIndex].ImagePercentage, NonOccImgPerc, (*Entities)[EntityIndex].OcclusionPercentage,
		//	(*Entities)[EntityIndex].bIsClipped);
	}
	else
	{
		if (!bSkelBoneActive)
		{
			// Set percentage  (non occ image perc - occ image perc / non occ image perc)
			const float ImgPercDiff = NonOccImgPerc - (*SkelEntities)[SkelIndex].ImagePercentage;
			const float OccPerc = ImgPercDiff / NonOccImgPerc;
			(*SkelEntities)[SkelIndex].OcclusionPercentage = OccPerc < 0.01f ? 0.f : OccPerc;

			// Set flag showing if the entity is clipped (touches the edge of the image)
			(*SkelEntities)[SkelIndex].bIsClipped = bIsClipped;

			//// DEBUG
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%s-%s] \t\t ImgPerc=%.8f; NonOccImgPerc=%.8f; OccPerc=%.8f; bIsClipped=%d;"),
			//	*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id,
			//	(*SkelEntities)[SkelIndex].ImagePercentage, NonOccImgPerc, (*SkelEntities)[SkelIndex].OcclusionPercentage,
			//	(*SkelEntities)[SkelIndex].bIsClipped);
		}
		else
		{
			// Set percentage  (non occ image perc - occ image perc / non occ image perc)
			const float ImgPercDiff = NonOccImgPerc - (*SkelEntities)[SkelIndex].Bones[BoneIndex].ImagePercentage;
			const float OccPerc = ImgPercDiff / NonOccImgPerc;
			(*SkelEntities)[SkelIndex].Bones[BoneIndex].OcclusionPercentage = OccPerc < 0.01f ? 0.f : OccPerc;

			// Set flag showing if the entity is clipped (touches the edge of the image)
			(*SkelEntities)[SkelIndex].Bones[BoneIndex].bIsClipped = bIsClipped;

			//// DEBUG
			//UE_LOG(LogTemp, Error, TEXT("%s::%d [%s-%s-%s] \t\t ImgPerc=%.8f; NonOccImgPerc=%.8f; OccPerc=%.8f; bIsClipped=%d;"),
			//	*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id, *(*SkelEntities)[SkelIndex].Bones[BoneIndex].Class,
			//	(*SkelEntities)[SkelIndex].Bones[BoneIndex].ImagePercentage, NonOccImgPerc, (*SkelEntities)[SkelIndex].Bones[BoneIndex].OcclusionPercentage,
			//	(*SkelEntities)[SkelIndex].Bones[BoneIndex].bIsClipped);
		}
	}	
}

// Output progress to terminal
void USLVisionOverlapCalc::PrintProgress() const
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t\t OverlapCalc=%ld/%ld;"),
		*FString(__func__), __LINE__, CurrOverlapCalcIdx + 1, TotalOverlapCalcNum);
}

/* Helper */
// Return INDEX_NONE if not possible
int32 USLVisionOverlapCalc::GetMaterialIndexOfCurrentlySelectedBone()
{
	if (SkelIndex == INDEX_NONE || BoneIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Skeletal data or bone data index is not set, cannot receive index without this.."), *FString(__func__), __LINE__);
		return INDEX_NONE;
	}
	//if (!CurrSkelDataComp)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Curr skeltal data is not set, cannot receive index without this.."), *FString(__func__), __LINE__);
	//	return INDEX_NONE;
	//}
	//if (int32* Idx = CurrSkelDataComp->BoneClassToMaterialIndex.Find((*SkelEntities)[SkelIndex].Bones[BoneIndex].Class))
	//{
	//	return *Idx;
	//}
	//else
	//{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find a material index mapping for bone class %s"),
			*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Bones[BoneIndex].Class);
		return INDEX_NONE;
	//}
}