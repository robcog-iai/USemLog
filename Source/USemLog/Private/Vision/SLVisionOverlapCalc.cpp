// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionOverlapCalc.h"
#include "Components/StaticMeshComponent.h"
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

		if (Parent && ViewportClient)
		{
			bIsInit = true;
		}		
	}
}

// 
void USLVisionOverlapCalc::Start(FSLVisionViewData* CurrViewData)
{
	if (!bIsStarted && bIsInit)
	{
		Entities = &CurrViewData->Entities;
		SkelEntities = &CurrViewData->SkelEntities;

		if (!SetupFirstItem())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No items found in the scene.."), *FString(__func__), __LINE__);
			return;
		}
		ApplyMaterial();


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

// 
void USLVisionOverlapCalc::Finish()
{
	if (!bIsFinished && bIsStarted)
	{
		EntityIndex = INDEX_NONE;
		SkelIndex = INDEX_NONE;
		bSkelArrayActive = false;
		
		Entities = nullptr;
		SkelEntities = nullptr;

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

	// Check if there are any other items in the scene
	if (SetupNextItem())
	{
		ApplyMaterial();
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


// 
bool USLVisionOverlapCalc::SetupFirstItem()
{
	if (SetupFirstEntity())
	{
		return true;
	}
	else if (SetupFirstSkel())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// 
bool USLVisionOverlapCalc::SetupNextItem()
{
	if (!bSkelArrayActive)
	{
		if (SetupNextEntity())
		{
			return true;
		}
		else
		{
			return SetupFirstSkel();
		}
	}
	else
	{
		return SetupNextSkel();
	}
}

// 
bool USLVisionOverlapCalc::SetupFirstEntity()
{
	if (EntityIndex == INDEX_NONE && Entities && Entities->Num() > 0)
	{
		EntityIndex = 0;
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no entity array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// 
bool USLVisionOverlapCalc::SetupNextEntity()
{
	if (EntityIndex != INDEX_NONE)
	{
		EntityIndex++;
		if (!Entities->IsValidIndex(EntityIndex))
		{
			EntityIndex = INDEX_NONE;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d First entity was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

// 
bool USLVisionOverlapCalc::SetupFirstSkel()
{
	if (SkelIndex == INDEX_NONE && SkelEntities && SkelEntities->Num() > 0)
	{		
		SkelIndex = 0;
		bSkelArrayActive = true;
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no skel array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// 
bool USLVisionOverlapCalc::SetupNextSkel()
{
	if (SkelIndex != INDEX_NONE)
	{
		SkelIndex++;
		if (!SkelEntities->IsValidIndex(SkelIndex))
		{
			SkelIndex = INDEX_NONE;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d First skel was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

//
void USLVisionOverlapCalc::ApplyMaterial()
{
	if (!bSkelArrayActive)
	{
		if (AStaticMeshActor* SMAClone = Parent->GetStaticMeshMaskClone((*Entities)[EntityIndex].Id))
		{
			// Clear any previously cached materials
			/*CachedMaterials.Empty();
			CachedMaterials = SMAClone->GetStaticMeshComponent()->GetMaterials();*/

			UE_LOG(LogTemp, Error, TEXT("%s::%d Clone %s: %s - %s;"),
				*FString(__func__), __LINE__, *SMAClone->GetName(),
				*(*Entities)[EntityIndex].Class,
				*(*Entities)[EntityIndex].Id);
			/*FLinearColor LC = FLinearColor::Blue;
			SMAClone->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(FName("MaskColorParam"), LC);*/

			//for (auto & MatInterf : SMAClone->GetStaticMeshComponent()->GetMaterials())
			//{
			//	UMaterial* Mat = MatInterf->GetMaterial();
			//	FLinearColor LC = FLinearColor::Blue;
			//	Mat->SetVectorParameterValueEditorOnly(FName("MaskColorParam"), LC);
			//	//Mat->GetVectorParameterValue(FName("MaskColorParam"), LC);
			//	Mat->bDisableDepthTest;
			//}

			(*Entities)[EntityIndex].OcclusionPercentage = 0.69f;
			(*Entities)[EntityIndex].bClipped = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity %s - %s;"),
				*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
		}




	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Active item= %s - %s;"),
			*FString(__func__), __LINE__, *(*SkelEntities)[SkelIndex].Class, *(*SkelEntities)[SkelIndex].Id);
	}
	//FColor MaskColor = MaskToEntity[0].Key;
	//DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
	//MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
	//DefaultMaskMaterial->bDisableDepthTest
}
