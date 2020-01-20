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

#include "SLSkeletalDataComponent.h"

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
void USLMaskCalibrationTool::Init(bool bOnlyDemo, const FString& InFolderName)
{
	if (!bIsInit)
	{
		IncludeLocallyFolderName = InFolderName;

		// Create mask clones of the available entities, hide everything else
		//if (!SetupWorld(bOnlyDemo))
		if (bOnlyDemo)
		{
			SetupWorld(bOnlyDemo);
			return;
		}

		if(!LoadMaskMappings())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No entities with visual masks loaded.."), *FString(__func__), __LINE__);
			return;
		}

		// Spawn helper actors, items and scan poses, skip items larger that the distance to camera
		if (!LoadTargetCameraPoseActor())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load a camera dummy mesh for moving the target view.."), *FString(__func__), __LINE__);
			return;
		}

		// Set the screenshot resolution;
		InitScreenshotResolution(FIntPoint(320,240));

		// Set rendering parameters
		InitRenderParameters();

		// Bind the screenshot callback
		ViewportClient = GetWorld()->GetGameViewport();
		if (!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLMaskCalibrationTool::ScreenshotCB);

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLMaskCalibrationTool::Start()
{
	if (!bIsStarted && bIsInit)
	{
		
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			// Hide default pawn
			PC->GetPawnOrSpectator()->SetActorHiddenInGame(true);

			// Set view mode to unlit
			PC->ConsoleCommand("viewmode unlit");

			// Set view target to dummy camera
			PC->SetViewTarget(CameraPoseActor);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot access the first player controller"), *FString(__func__), __LINE__);
			return;
		}		


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

// Move first item in position
bool USLMaskCalibrationTool::SetupFirstItem()
{
	return false;
}

// Move thenext item in position, return false if there are no more items
bool USLMaskCalibrationTool::SetupNextItem()
{
	return false;
}


// Init hi-res screenshot resolution
void USLMaskCalibrationTool::InitScreenshotResolution(FIntPoint InResolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(InResolution.X, InResolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;
}

// Init render parameters (resolution, view mode)
void USLMaskCalibrationTool::InitRenderParameters()
{
	// Defines the memory layout used for the GBuffer,
	// 0: lower precision (8bit per component, for profiling), 1: low precision (default)
	// 3: high precision normals encoding, 5: high precision
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.GBufferFormat"))->Set(5);


	// Set the near clipping plane (in cm)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetNearClipPlane"))->Set(0); // Not a console variable, but a command
	//GNearClippingPlane = 0; // View is distorted after finishing the scanning
#if WITH_EDITOR
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
	}
#endif // WITH_EDITOR

	//// AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	//// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Load mesh that will be used to render all the mask colors on screen
bool USLMaskCalibrationTool::LoadMaskRenderMesh()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_MaskRenderDummy");
	MaskRenderActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector(300.f, 0.f, 0.f)), SpawnParams);
	if (!MaskRenderActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	MaskRenderActor->SetActorLabel(FString(TEXT("SM_MaskRenderDummy")));
#endif // WITH_EDITOR

	UStaticMesh* DummyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/USemLog/Vision/MaskRenderDummy/SM_MaskRenderDummy.SM_MaskRenderDummy"));
	if (!DummyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find mask render dummy mesh.."), *FString(__func__), __LINE__);
		MaskRenderActor->Destroy();
		return false;
	}
	MaskRenderActor->GetStaticMeshComponent()->SetStaticMesh(DummyMesh);
	MaskRenderActor->SetMobility(EComponentMobility::Movable);


	return true;
}

// Load scan camera convenience actor
bool USLMaskCalibrationTool::LoadTargetCameraPoseActor()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_ScanCameraPoseDummy");
	CameraPoseActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), 
		FTransform(FRotator::ZeroRotator, FVector::ZeroVector), SpawnParams);
	if (!CameraPoseActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	CameraPoseActor->SetActorLabel(FString(TEXT("SM_ScanCameraPoseDummy")));
#endif // WITH_EDITOR

	UStaticMesh* CameraPoseDummyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/USemLog/Vision/ScanCameraPoseDummy/SM_ScanCameraPoseDummy.SM_ScanCameraPoseDummy"));
	if (!CameraPoseDummyMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find camera pose dummy mesh.."), *FString(__func__), __LINE__);
		CameraPoseActor->Destroy();
		return false;
	}
	CameraPoseActor->GetStaticMeshComponent()->SetStaticMesh(CameraPoseDummyMesh);
	CameraPoseActor->SetMobility(EComponentMobility::Movable);

	return true;
}

// Load the mask colors to their entities mapping
bool USLMaskCalibrationTool::LoadMaskMappings()
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Hide and disable physics on all actors by default
		ActItr->SetActorHiddenInGame(true);

		/*  Static mesh actors */
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*ActItr))
		{
			// Get the mask color, will be black if it does not exist
			FColor MaskColor(FColor::FromHex(FTags::GetValue(SMA, "SemLog", "VisMask")));
			if (MaskColor == FColor::Black)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask.."), *FString(__func__), __LINE__, *ActItr->GetName());
				continue;
			}

			// Temporally store the actors and their designated mask color
			MaskToEntity.Emplace(MaskColor, SMA);
		}

		/* Skeletal mesh actors */
		else if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(*ActItr))
		{
			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if (UActorComponent* AC = SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
				
				for (const auto& BoneDataPair : SkDC->SemanticBonesData)
				{
					FName BoneName = BoneDataPair.Key;
					FSLBoneData BoneData = BoneDataPair.Value;

					// Get the mask color, will be black if it does not exist
					FColor MaskColor(FColor::FromHex(BoneData.VisualMask));
					if (MaskColor == FColor::Black)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d %s - %s has no visual mask.."), *FString(__func__), __LINE__,
							*ActItr->GetName(), *BoneName.ToString());
						continue;
					}
					
					MaskToSkeletalBone.Emplace(MaskColor, MakeTuple(SkMA, BoneName));					
				}
			}
		}
	}

	return MaskToEntity.Num() > 0 || MaskToSkeletalBone.Num() > 0;
}


/* Legacy */
// Create mask clones of the available entities, hide everything else
bool USLMaskCalibrationTool::SetupWorld(bool bOnlyDemo)
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
	TMap<FColor, AStaticMeshActor*> TempColorToSMA;
	TMap<USLSkeletalDataComponent*, ASkeletalMeshActor*> TempDataToSkMA;

	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Hide and disable physics on all actors by default
		ActItr->SetActorHiddenInGame(true);
		ActItr->DisableComponentsSimulatePhysics();
		ActItr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		/*  Static mesh actors */
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*ActItr))
		{
			// Get the mask color, will be black if it does not exist
			FColor MaskColor(FColor::FromHex(FTags::GetValue(SMA, "SemLog", "VisMask")));
			if (MaskColor == FColor::Black)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask.."), *FString(__func__), __LINE__, *ActItr->GetName());
				continue;
			}

			// Temporally store the actors and their designated mask color
			TempColorToSMA.Emplace(MaskColor, SMA);
		}

		/* Skeletal mesh actors */
		else if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(*ActItr))
		{
			// Get the semantic data component containing the semantics (class names mask colors) about the bones
			if (UActorComponent* AC = SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
				TempDataToSkMA.Emplace(SkDC, SkMA);
			}
		}
	}

	// Create clones from the sm actors
	for (const auto& Pair : TempColorToSMA)
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
		SMAClone->SetActorHiddenInGame(!bOnlyDemo);
		CloneToRealArray.Emplace(SMAClone, SMA);
	}

	// Create clones from the skel actors
	if (bOnlyDemo)
	{
		for (const auto& DataToSkPair : TempDataToSkMA)
		{
			USLSkeletalDataComponent* SkData = DataToSkPair.Key;
			ASkeletalMeshActor* SkMA = DataToSkPair.Value;

			// Create a clone of the actor
			FActorSpawnParameters Parameters;
			Parameters.Template = SkMA;
			Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			//Parameters.Instigator = SMA->GetInstigator();
			Parameters.Name = FName(*(SkMA->GetName() + TEXT("_MaskClone")));
			ASkeletalMeshActor* SkMAClone = GetWorld()->SpawnActor<ASkeletalMeshActor>(SkMA->GetClass(), Parameters);

			for (auto& NameToBoneDataPair : SkData->SemanticBonesData)
			{
				// Check if bone class and visual mask is set
				if (NameToBoneDataPair.Value.IsClassSet())
				{
					// Get the mask color, will be black if it does not exist
					FColor SemColor(FColor::FromHex(NameToBoneDataPair.Value.VisualMask));
					if (SemColor == FColor::Black)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d %s --> %s has no visual mask, setting to black.."),
							*FString(__func__), __LINE__, *SkMA->GetName(), *NameToBoneDataPair.Value.Class);
					}

					// Create the mask material with the color
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
						FLinearColor::FromSRGBColor(FColor::FromHex(NameToBoneDataPair.Value.VisualMask)));
					SkMAClone->GetSkeletalMeshComponent()->SetMaterial(NameToBoneDataPair.Value.MaskMaterialIndex, DynamicMaskMaterial);
				}
			}

			// Hide all mask clones actors initailly by default
			SkMAClone->SetActorHiddenInGame(!bOnlyDemo);
		}
	}


	if (CloneToRealArray.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No mask clones where generated.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

