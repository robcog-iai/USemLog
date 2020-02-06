// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLMaskCalibrationTool.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMesh.h"
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

#if WITH_EDITOR
#include "Editor.h"
#endif // WITH_EDITOR

// Ctor
USLMaskCalibrationTool::USLMaskCalibrationTool()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	CurrEntityIdx = INDEX_NONE;
	CurrSkelIdx = INDEX_NONE;
	bSkelMasksActive = false;
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
void USLMaskCalibrationTool::Init(bool bOverwrite, bool bOnlyDemo, const FString& InFolderName)
{
	if (!bIsInit)
	{
		IncludeLocallyFolderName = InFolderName;
		//IncludeLocallyFolderName = "CalibrationImages";

		// Create mask clones of the available entities, hide everything else
		if (bOnlyDemo)
		{
			SetupMaskColorsWorld(bOnlyDemo);
			return;
		}

		// Load the mask colors to their entities mapping
		if (!LoadMaskMappings(bOverwrite))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No entities with visual masks loaded.."), *FString(__func__), __LINE__);
			return;
		}

		// Spawn helper actors, items and scan poses, skip items larger that the distance to camera
		if (!CreateMaskRenderMesh())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load a mesh dummy mesh for applying the mask colors.."), *FString(__func__), __LINE__);
			return;
		}

		// Spawn helper actors, items and scan poses, skip items larger that the distance to camera
		if (!CreateTargetCameraPoseActor())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load a camera dummy mesh for moving the target view.."), *FString(__func__), __LINE__);
			return;
		}

		// Set the screenshot resolution;
		InitScreenshotResolution(FIntPoint(160,120));

		// Set rendering parameters
		InitRenderParameters();

		// Bind the screenshot callback
		ViewportClient = GetWorld()->GetGameViewport();
		if (!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}

		// Bind screenshot callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLMaskCalibrationTool::ScreenshotCB);

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLMaskCalibrationTool::Start()
{
	if (!bIsStarted && bIsInit)
	{	
		// Make sure the spawned render actor is visible
		MaskRenderActor->SetActorHiddenInGame(false);

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			// Hide default pawn
			PC->GetPawnOrSpectator()->SetActorHiddenInGame(true);

			// Set view mode to unlit
			PC->ConsoleCommand("viewmode unlit");

			// Set view target to dummy camera
			PC->SetViewTarget(CameraPoseActor);

			// Set frist mask color
			if (!SetupFirstMaskColor())
			{
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot access the first player controller"), *FString(__func__), __LINE__);
			return;
		}		

		// Start the dominoes
		RequestScreenshot();

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
	// Request screenshot on game thread
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		if (CurrEntityIdx != INDEX_NONE)
		{
			CurrImgName = FString::FromInt(CurrEntityIdx) + "_" 
				+ MaskToEntity[CurrEntityIdx].Value->GetName() + "_" 
				+ MaskToEntity[CurrEntityIdx].Key.ToHex();
		}
		else if (CurrSkelIdx != INDEX_NONE)
		{
			CurrImgName = FString::FromInt(CurrSkelIdx) + "_" 
				+ MaskToSkelAndBone[CurrSkelIdx].Value.Key->GetName() + "_" 
				+ MaskToSkelAndBone[CurrSkelIdx].Value.Value.ToString() + "_"
				+ MaskToSkelAndBone[CurrSkelIdx].Key.ToHex();
		}
		else
		{
			CurrImgName = "NONE";
		}

		GetHighResScreenshotConfig().FilenameOverride = CurrImgName;
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when screenshot is captured
void USLMaskCalibrationTool::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{	
	// Get the actual rendered color
	FColor RenderedColor = GetRenderedColor(Bitmap);
	StoreRenderedColor(RenderedColor);

	// Save the png locally
	if (!IncludeLocallyFolderName.IsEmpty())
	{
		// Compress image
		TArray<uint8> CompressedBitmap;
		FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);
		FString Path = FPaths::ProjectDir() + "/SemLog/" + IncludeLocallyFolderName + "/Editor/" + CurrImgName + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	if (SetupNextMaskColor())
	{
		RequestScreenshot();
	}
	else
	{
		// No more masks
		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished.."),
			*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
		QuitEditor();
	}
}

// Render the color of te first mask
bool USLMaskCalibrationTool::SetupFirstMaskColor()
{
	if (SetupFirstEntityMaskColor())
	{
		return true;
	}
	else if (SetupFirstSkelMaskColor())
	{		
		return true;
	}
	else
	{
		return false;
	}
}

// Render the color of te next mask
bool USLMaskCalibrationTool::SetupNextMaskColor()
{
	if (!bSkelMasksActive)
	{
		if (SetupNextEntityMaskColor())
		{
			return true;
		}
		else 
		{
			return SetupFirstSkelMaskColor();
		}
	}
	else
	{
		return SetupNextSkelMaskColor();
	}
}

// Return the actual mask rendered color
FColor USLMaskCalibrationTool::GetRenderedColor(const TArray<FColor>& Bitmap)
{
	bool bRenderedColorIsSet = false;
	FColor RenderedColor;

	for (const auto& C : Bitmap)
	{
		if (C != FColor::Black)
		{
			if (bRenderedColorIsSet)
			{
				if (C != RenderedColor)
				{
					// Make sure no other nuances appear
					UE_LOG(LogTemp, Error, TEXT("%s::%d Different color nuance found %s/%s;"),
						*FString(__func__), __LINE__, *C.ToString(), *RenderedColor.ToString());
				}
			}
			else
			{
				RenderedColor = C;
				bRenderedColorIsSet = true;
			}
		}
	}
	return RenderedColor;
}

// Store the rendered color
bool USLMaskCalibrationTool::StoreRenderedColor(const FColor & InRenderedColor)
{
	// Store the color value
	if (!bSkelMasksActive)
	{
		FColor OriginalColor = MaskToEntity[CurrEntityIdx].Key;
		AActor* Parent = MaskToEntity[CurrEntityIdx].Value;
#if WITH_EDITOR
		// Apply the changes in the editor world
		if (AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(Parent))
		{
			FTags::AddKeyValuePair(EdAct, "SemLog", "RenderedVisMask", InRenderedColor.ToHex());
			PrintProgress(Parent, OriginalColor, InRenderedColor);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
				*FString(__func__), __LINE__, *Parent->GetName());
		}
#endif // WITH_EDITOR
	}
	else
	{
		FColor OriginalColor = MaskToSkelAndBone[CurrSkelIdx].Key;
		AActor* Parent = MaskToSkelAndBone[CurrSkelIdx].Value.Key;
		FName BoneName = MaskToSkelAndBone[CurrSkelIdx].Value.Value;
#if WITH_EDITOR
		// Apply the changes in the editor world
		if (AActor* EdAct = EditorUtilities::GetEditorWorldCounterpartActor(Parent))
		{
			if (UActorComponent* AC = EdAct->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
			{
				USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
				if (FSLBoneData* BoneData = SkDC->SemanticBonesData.Find(BoneName))
				{
					BoneData->RenderedVisualMask = InRenderedColor.ToHex();
					PrintProgress(Parent, OriginalColor, InRenderedColor, BoneName.ToString());
					return true;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access %s in editor world.."),
				*FString(__func__), __LINE__, *Parent->GetName());
		}
#endif // WITH_EDITOR
	}
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
//#if WITH_EDITOR
//	if (GEngine)
//	{
//		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
//	}
//#endif // WITH_EDITOR

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
bool USLMaskCalibrationTool::CreateMaskRenderMesh()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SM_MaskRenderDummy");
	MaskRenderActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector(150.f, 0.f, 0.f)), SpawnParams);
	if (!MaskRenderActor)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn convenience camera pose actor.."), *FString(__func__), __LINE__);
		return false;
	}
#if WITH_EDITOR
	MaskRenderActor->SetActorLabel(FString(TEXT("SM_MaskRenderDummy")));
#endif // WITH_EDITOR

	MaskRenderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/USemLog/Vision/MaskRenderDummy/SM_MaskRenderDummy.SM_MaskRenderDummy"));
	if (!MaskRenderMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find mask render mask mesh.."), *FString(__func__), __LINE__);
		MaskRenderActor->Destroy();
		return false;
	}
	MaskRenderActor->GetStaticMeshComponent()->SetStaticMesh(MaskRenderMesh);
	MaskRenderActor->SetMobility(EComponentMobility::Movable);

	//UMaterialInterface* DefaultMaskMaterial = MaskRenderMesh->GetMaterial(0);
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
		TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."), *FString(__func__), __LINE__);
		return false;
	}
	//DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;
		
	DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::White);

	MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);

	return true;
}

// Load scan camera convenience actor
bool USLMaskCalibrationTool::CreateTargetCameraPoseActor()
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
bool USLMaskCalibrationTool::LoadMaskMappings(bool bOverwrite)
{
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Hide and disable physics on all actors by default
		ActItr->SetActorHiddenInGame(true);

		/*  Static mesh actors */
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*ActItr))
		{
			FString MaskStr = FTags::GetValue(SMA, "SemLog", "VisMask");
			if (!MaskStr.IsEmpty())
			{
				if (!bOverwrite && FTags::HasKey(SMA, "SemLog", "RenderedVisMask"))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s; Already has a rendered visual mask, skipping.."),
						*FString(__func__), __LINE__, *ActItr->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s; VisualMask=%s;"), *FString(__func__), __LINE__, *ActItr->GetName(), *MaskStr);
					FColor MaskColor(FColor::FromHex(MaskStr));
					MaskToEntity.Emplace(MakeTuple(MaskColor, SMA));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask.."), *FString(__func__), __LINE__, *ActItr->GetName());
			}
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
					FString MaskStr = BoneDataPair.Value.VisualMask;

					if (!MaskStr.IsEmpty())
					{
						if (!bOverwrite && !BoneDataPair.Value.RenderedVisualMask.IsEmpty())
						{
							UE_LOG(LogTemp, Warning, TEXT("%s::%d %s - %s; Already has a rendered visual mask, skipping.."),
								*FString(__func__), __LINE__, *ActItr->GetName(), *BoneName.ToString());
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("%s::%d %s - %s; VisualMask=%s;"), *FString(__func__), __LINE__,
								*ActItr->GetName(), *BoneName.ToString(), *MaskStr);
							FColor MaskColor(FColor::FromHex(MaskStr));
							MaskToSkelAndBone.Emplace(MakeTuple(MaskColor, MakeTuple(SkMA, BoneName)));
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d %s - %s has no visual mask.."), *FString(__func__), __LINE__,
							*ActItr->GetName(), *BoneName.ToString());
						continue;
					}					
				}
			}
		}
	}

	return MaskToEntity.Num() > 0 || MaskToSkelAndBone.Num() > 0;
}

// Render the color of te next entity mask
bool USLMaskCalibrationTool::SetupFirstEntityMaskColor()
{
	if (CurrEntityIdx == INDEX_NONE && MaskToEntity.Num() > 0)
	{
		CurrEntityIdx = 0;
		FColor MaskColor = MaskToEntity[0].Key;
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
		MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
		return true;
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no entity mask array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Render the color of te next entity mask
bool USLMaskCalibrationTool::SetupNextEntityMaskColor()
{
	if (CurrEntityIdx != INDEX_NONE)
	{
		CurrEntityIdx++;
		if (!MaskToEntity.IsValidIndex(CurrEntityIdx))
		{
			CurrEntityIdx = INDEX_NONE;
			return false;
		}
		else
		{
			FColor MaskColor = MaskToEntity[CurrEntityIdx].Key;
			DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
			MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
			return true;
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d First entity was not set before.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Render the color of te first skeletal mask
bool USLMaskCalibrationTool::SetupFirstSkelMaskColor()
{
	if (CurrSkelIdx == INDEX_NONE && MaskToSkelAndBone.Num() > 0)
	{
		CurrSkelIdx = 0;
		FColor MaskColor = MaskToSkelAndBone[0].Key;
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
		MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
		bSkelMasksActive = true;
		return true;
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Already set, or no skel mask array loaded.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Render the color of te next skeletal mask
bool USLMaskCalibrationTool::SetupNextSkelMaskColor()
{
	if (CurrSkelIdx != INDEX_NONE)
	{
		CurrSkelIdx++;
		if (!MaskToSkelAndBone.IsValidIndex(CurrSkelIdx))
		{
			CurrSkelIdx = INDEX_NONE;
			return false;
		}
		else
		{
			FColor MaskColor = MaskToSkelAndBone[CurrSkelIdx].Key;
			DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));
			MaskRenderMesh->SetMaterial(0, DynamicMaskMaterial);
			return true;
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d Not set yet.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Output progress to terminal
void USLMaskCalibrationTool::PrintProgress(AActor* Parent, FColor OrigColor, FColor RenderedColor, FString BoneName) const
{
	if (!BoneName.IsEmpty())
	{
		BoneName.InsertAt(0, "- ");
	}

	int32 ManhattanDistance = FMath::Abs((OrigColor.R - RenderedColor.R)) + FMath::Abs((OrigColor.G - RenderedColor.G)) + FMath::Abs((OrigColor.B - RenderedColor.B));
	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t[%2.f] \t%s %s: t%s\t->\t%s; \tManDis=%ld;"),
		*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds(),
		*Parent->GetName(), *BoneName,
		*OrigColor.ToString(), *RenderedColor.ToString(),
		ManhattanDistance);
}

// Quit the editor once the scanning is finished
void USLMaskCalibrationTool::QuitEditor()
{
	//FGenericPlatformMisc::RequestExit(false);
	//
	//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
	//FPlatformMisc::RequestExit(0);

#if WITH_EDITOR	
	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
#endif // WITH_EDITOR
}


/* Legacy */
// Create mask clones of the available entities, hide everything else
bool USLMaskCalibrationTool::SetupMaskColorsWorld(bool bOnlyDemo)
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
		UMaterialInstanceDynamic* LocalDynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		LocalDynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(MaskColor));

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
				SMC->SetMaterial(MatIdx, LocalDynamicMaskMaterial);
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
					UMaterialInstanceDynamic* LocalDynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					LocalDynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
						FLinearColor::FromSRGBColor(FColor::FromHex(NameToBoneDataPair.Value.VisualMask)));
					SkMAClone->GetSkeletalMeshComponent()->SetMaterial(NameToBoneDataPair.Value.MaskMaterialIndex, LocalDynamicMaskMaterial);
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

