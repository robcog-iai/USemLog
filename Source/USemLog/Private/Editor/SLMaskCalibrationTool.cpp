// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLMaskCalibrationTool.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FileHelper.h"
#include "Async.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

// UUtils
#include "Tags.h"

// Ctor
USLMaskCalibrationTool::USLMaskCalibrationTool()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLMaskCalibrationTool::~USLMaskCalibrationTool()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();
	}
}


// Init scanner
void USLMaskCalibrationTool::Init(const FString& InFolderName)
{
	if (!bIsInit)
	{
		IncludeLocallyFolderName = InFolderName;

		// If no view modes are available, add a default one
		if (SetupWorld())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No entities with visual masks loaded.."), *FString(__func__), __LINE__);
			return;
		}

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLMaskCalibrationTool::Start()
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// Finish scanning
void USLMaskCalibrationTool::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Request a screenshot
void USLMaskCalibrationTool::RequestScreenshot()
{

}

// Called when screenshot is captured
void USLMaskCalibrationTool::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{

}


// Create mask clones of the available entities, hide everything else
bool USLMaskCalibrationTool::SetupWorld()
{
	// Load the default mask material
	// this will be used as a template to create the colored mask materials to add to the clones
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
		TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."), *FString(__func__), __LINE__);
		return false;
	}
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Temp arrays to store the entities with visual masks 
	// avoids spawning in the for loop which ends up in an infinite loop due to newly added actors
	TMap<FColor, AStaticMeshActor*> TempEntities;
	
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Hide and disable physics on all actors by default
		ActItr->SetActorHiddenInGame(true);
		ActItr->DisableComponentsSimulatePhysics();

		/*  Static mesh actors */
		if(AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*ActItr))
		{
			// Get the mask color, will be black if it does not exist
			FColor MaskColor(FColor::FromHex(FTags::GetValue(SMA, "SemLog", "VisMask")));
			if (MaskColor == FColor::Black)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask.."), *FString(__func__), __LINE__, *ActItr->GetName());
				continue;
			}

			// Temporally store the actors and their designated mask color
			TempEntities.Emplace(MaskColor, SMA);
		}

		/* Skeletal mesh actors */
		else if (AStaticMeshActor* SkMA = Cast<AStaticMeshActor>(*ActItr))
		{
		}
	}

	// Create clones from the actors
	for(const auto& Pair : TempEntities)
	{
		const FColor MaskColor = Pair.Key;
		AStaticMeshActor* SMA = Pair.Value;

		// Create the mask material with the color
		UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));

		// Create a clone of the actor
		FActorSpawnParameters Parameters;
		Parameters.Template = SMA;
		Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		//Parameters.Instigator = SMA->GetInstigator();
		Parameters.Name = FName(*(SMA->GetName() + TEXT("_MaskClone")));
		AStaticMeshActor* SMAClone = GetWorld()->SpawnActor<AStaticMeshActor>(SMA->GetClass(), Parameters);

		// Apply mask color
		if (UStaticMeshComponent* SMC = SMAClone->GetStaticMeshComponent())
		{
			for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
			{
				SMC->SetMaterial(MatIdx, DynamicMaskMaterial);
			}
		}

		// Hide all mask clones actors initailly by default
		SMAClone->SetActorHiddenInGame(false);
		EntityClones.Emplace(MaskColor, SMA);
	}

	if (EntityClones.Num() == 0 /* SkelClones.Num() == 0 */)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No mask clones where generated.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}
