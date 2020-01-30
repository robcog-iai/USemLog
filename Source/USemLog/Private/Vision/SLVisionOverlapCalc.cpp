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
#include "SLVisionLogger.h"


// Constructor
USLVisionOverlapCalc::USLVisionOverlapCalc() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	EntityIndex = INDEX_NONE;
	SkelIndex = INDEX_NONE;
	CurrSMAClone = nullptr;
	CurrPMAClone = nullptr;
	bSkelArrayActive = false;
}

// Destructor
USLVisionOverlapCalc::~USLVisionOverlapCalc()
{
}

// Give control to the overlap calc to pause and start its parent (vision logger)
void USLVisionOverlapCalc::Init(USLVisionLogger* InParent, FIntPoint InResolution)
{
	if (!bIsInit)
	{
		Parent = InParent;
		ViewportClient = GetWorld()->GetGameViewport();
		Resolution = InResolution / 4;
		SaveLocallyFolderName = "OverlapTest";

		// Load the default mask material
		// this will be used as a template to create the non-occluding mask materials to add to the clones
		DefaultNonOccludingMaterial = LoadObject<UMaterial>(this,
			TEXT("/USemLog/Vision/M_SLNonOccludingDefaultMask.M_SLNonOccludingDefaultMask"));
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
void USLVisionOverlapCalc::Start(FSLVisionViewData* CurrViewData)
{
	if (!bIsStarted && bIsInit)
	{
		Entities = &CurrViewData->Entities;
		SkelEntities = &CurrViewData->SkelEntities;

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
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		CurrImageFilename = "OverlapTest_" + FString::SanitizeFloat(GetWorld()->GetTimeSeconds()) + "_" + FString::FromInt(EntityIndex);
		GetHighResScreenshotConfig().FilenameOverride = CurrImageFilename;
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when the screenshot is captured
void USLVisionOverlapCalc::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	// Save the png locally
	if (!SaveLocallyFolderName.IsEmpty())
	{
		// Compress image
		TArray<uint8> CompressedBitmap;
		FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);
		FString Path = FPaths::ProjectDir() + "/SemLog/" + SaveLocallyFolderName + "/" + CurrImageFilename + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	// Re-apply original material before selecting the next item
	ReApplyOriginalMaterial();

	// Check if there are any other items in the scene
	if (SelectNextItem())
	{
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
		return SelectNextSkel();
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no entity array loaded.."), *FString(__func__), __LINE__);
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
		UE_LOG(LogTemp, Error, TEXT("%s::%d First entity was not set before.."), *FString(__func__), __LINE__);
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
		CurrPMAClone = Parent->GetPoseableSkeletalMaskCloneFromId((*SkelEntities)[SkelIndex].Id);
		if (!CurrPMAClone)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to skel entity %s - %s, continuing.."),
				*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
			return SelectNextSkel();
		}
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no skel array loaded.."), *FString(__func__), __LINE__);
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
			CurrPMAClone = Parent->GetPoseableSkeletalMaskCloneFromId((*SkelEntities)[SkelIndex].Id);
			if (!CurrPMAClone)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find pointer to skel entity %s - %s, continuing.."),
					*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
				return SelectNextSkel();
			}
			return true;;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d First skel was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Apply the non occluding material to the currently selected item
void USLVisionOverlapCalc::ApplyNonOccludingMaterial()
{
	if (!bSkelArrayActive)
	{
		if (CurrSMAClone)
		{
			if (UStaticMeshComponent* MC = CurrSMAClone->GetStaticMeshComponent())
			{
				// Create the non-occluding material
				UMaterialInstanceDynamic* OccludingDynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultNonOccludingMaterial, GetTransientPackage());
				OccludingDynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::Red);
				
				int32 TotalNumMaterials = MC->GetNumMaterials();
				if (TotalNumMaterials > 0)
				{
					// Set array length
					CachedMaterials.AddZeroed(TotalNumMaterials);
					for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
					{
						// Cache material
						CachedMaterials[MaterialIndex] = MC->GetMaterial(MaterialIndex);

						// Switch material
						MC->SetMaterial(MaterialIndex, OccludingDynamicMaskMaterial);
					}
				}

				//CachedMaterials = MC->GetMaterials();

				//for (auto& MI : MC->GetMaterials())
				//{
				//	MI->GetMaterial()->bDisableDepthTest = true;					
				//}
				//MC->MarkRenderStateDirty();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."),	*FString(__func__), __LINE__);
		}
	}
	else
	{
		if (CurrPMAClone)
		{		
			if (UPoseableMeshComponent* PMC = CurrPMAClone->GetPoseableMeshComponent())
			{
				// Create the non-occluding material
				UMaterialInstanceDynamic* OccludingDynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultNonOccludingMaterial, GetTransientPackage());
				OccludingDynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::Red);

				int32 TotalNumMaterials = PMC->GetNumMaterials();
				if (TotalNumMaterials > 0)
				{
					// Set array length
					CachedMaterials.AddZeroed(TotalNumMaterials);
					for (int32 MaterialIndex = 0; MaterialIndex < TotalNumMaterials; ++MaterialIndex)
					{
						// Cache material
						CachedMaterials[MaterialIndex] = PMC->GetMaterial(MaterialIndex);

						// Switch material
						PMC->SetMaterial(MaterialIndex, OccludingDynamicMaskMaterial);
					}
				}

				//CachedMaterials = PMC->GetMaterials();

				//for (auto& MI : PMC->GetMaterials())
				//{
				//	MI->GetMaterial()->bDisableDepthTest = true;					
				//}
				//PMC->MarkRenderStateDirty();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
	//FColor MaskColor = MaskToEntity[0].Key;
	//DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
	//MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
	//DefaultMaskMaterial->bDisableDepthTest
}

// Re-apply the original material to the currently selected item
void USLVisionOverlapCalc::ReApplyOriginalMaterial()
{
	if (!bSkelArrayActive)
	{
		if (CurrSMAClone)
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
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
	else
	{
		if (CurrPMAClone)
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
			UE_LOG(LogTemp, Error, TEXT("%s::%d This should not happen.."), *FString(__func__), __LINE__);
		}
	}
}