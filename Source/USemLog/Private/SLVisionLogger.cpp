// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionLogger.h"
#include "Vision/SLVisionToolkit.h"
#include "Vision/SLVisionPoseableMeshActor.h"
#include "SLEntitiesManager.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"

// UUtils
#include "Conversions.h"
#include "Tags.h"

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	ViewModes.Add(ESLVisionLoggerViewMode::Color);
	//ViewModes.Add(ESLVisionLoggerViewMode::Unlit);
	ViewModes.Add(ESLVisionLoggerViewMode::Mask);
	//ViewModes.Add(ESLVisionLoggerViewMode::Depth);
	//ViewModes.Add(ESLVisionLoggerViewMode::Normal);
	CurrViewModeIdx = INDEX_NONE;
	CurrVirtualCameraIdx = INDEX_NONE;
	CurrTimestamp = -1.f;
	PrevViewMode = ESLVisionLoggerViewMode::NONE;
}

// Destructor
USLVisionLogger::~USLVisionLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}

	// Disconnect and clean db connection
	Disconnect();
}

// Init Logger
void USLVisionLogger::Init(const FString& InTaskId, const FString& InEpisodeId, const FString& InServerIp, uint16 InServerPort,
	bool bOverwriteVisionData, const FSLVisionLoggerParams& Params)
{
	if (!bIsInit)
	{
		Resolution = Params.Resolution;

		// Save the folder name if the images are going to be stored locally as well
		if(Params.bIncludeLocally)
		{
			TaskId = InTaskId;
		}
		
		// Init the semantic instances
		FSLEntitiesManager::GetInstance()->Init(GetWorld());

		// Set the captured image resolution
		InitScreenshotResolution(Params.Resolution);

		// Set rendering parameters
		InitRenderParameters();

		// Disable physics on all entities and make sure they are movable
		InitWorldEntities();

		// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
		CreatePoseableMeshes();

		// Connect to the database
		if(!Connect(InTaskId, InEpisodeId, InServerIp, InServerPort, bOverwriteVisionData))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not connect to the DB.."), *FString(__func__), __LINE__);
			return;
		}

		// Load the episode data (the poseable meshes should be setup before) 
		if(!LoadEpisode(Params.UpdateRate))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not download the episode data.."), *FString(__func__), __LINE__);
			return;
		}

		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found.."), *FString(__func__), __LINE__);
			return;
		}

		// Make sure the semantic entities are init before
		if(!LoadVirtualCameras())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No virtual cameras found.."), *FString(__func__), __LINE__);
			return;
		}

		// Used for the screenshot requests
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}
		// Bind the screenshot captured callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLVisionLogger::ScreenshotCB);

		// Create clones of every visible entity with the mask color
		if(ViewModes.Contains(ESLVisionLoggerViewMode::Mask))
		{
			if(!CreateMaskClones())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create mask clones, removing view type.."), *FString(__func__), __LINE__);
				ViewModes.Remove(ESLVisionLoggerViewMode::Mask);
			}
		}
		
		bIsInit = true;
	}
}

// Start logger
void USLVisionLogger::Start(const FString& EpisodeId)
{
	if (!bIsStarted && bIsInit)
	{
		// Hide default pawn from scene
		GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);
		
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
		if(!SetupFirstEpisodeFrame())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup the first world frame.."), *FString(__func__), __LINE__);
			return;
		}
		
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr at this point
		if(!GotoFirstCameraView())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup first camera view.."), *FString(__func__), __LINE__);
			return;
		}

		// Set the first render type
		if(!SetupFirstViewMode())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup first view mode.."), *FString(__func__), __LINE__);
			return;
		}

		CurrFrameData.Init(CurrTimestamp, Resolution);
		CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());
		
		// Start the dominoes
		RequestScreenshot();
		
		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLVisionLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Index the entries in the db
		CreateIndexes();

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Trigger the screenshot on the game thread
void USLVisionLogger::RequestScreenshot()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		//// FrameNum_CameraName_ViewMode
		//CurrImageFilename = FString::FromInt(EpisodeData.GetActiveFrameNum()) + "_" + 
		//	VirtualCameras[CurrCameraIdx]->GetClassName() + "_" + CurrViewModePostfix;
		
		// FrameNum_CameraNum_s-ms_Viewmode
		//CurrImageFilename = FString::FromInt(Episode.GetCurrIndex()) + "_" + 
		//	FString::FromInt(CurrVirtualCameraIdx) + "_" + 
		//	FString::SanitizeFloat(CurrTimestamp).Replace(TEXT("."),TEXT("-")) + "_" + CurrViewModePostfix;

		// FrameNum_CameraNum_Viewmode
		CurrImageFilename = FString::FromInt(Episode.GetCurrIndex()) + "_" + 
			FString::FromInt(CurrVirtualCameraIdx) + "_" + CurrViewModePostfix;
		
		GetHighResScreenshotConfig().FilenameOverride = CurrImageFilename;
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when the screenshot is captured
void USLVisionLogger::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	// Terminal output with the log progress
	PrintProgress();

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);

	// Check if the image should be saved locally as well
	if(!TaskId.IsEmpty())
	{
		SaveImageLocally(CompressedBitmap);
	}

	// Add the image and the view mode to the array
	CurrViewData.Images.Emplace(GetViewModeName(ViewModes[CurrViewModeIdx]), CompressedBitmap);

	// If mask mode is currently active, read out the entity data as well
	if(ViewModes[CurrViewModeIdx] == ESLVisionLoggerViewMode::Mask)
	{
		FSLVisionViewEntitiyData Entity("anId", "aClass");
		CurrViewData.Entities.Emplace(Entity);
		Entity.Class = "aClass2";
		Entity.Id = "anId2";
		CurrViewData.Entities.Emplace(Entity);

		FSLVisionViewSkelData Skel("aSkelId","sSkelClass");
		Skel.Bones.Emplace(FSLVisionViewSkelBoneData("ABoneName"));
		Skel.Bones.Emplace(FSLVisionViewSkelBoneData("ABoneName2"));
		CurrViewData.SkelEntities.Emplace(Skel);

		FSLVisionViewSkelData Skel2("aSkelId2", "sSkelClass2");
		Skel.Bones.Emplace(FSLVisionViewSkelBoneData("ABoneName2"));
		Skel.Bones.Emplace(FSLVisionViewSkelBoneData("ABoneName22"));
		CurrViewData.SkelEntities.Emplace(Skel2);
	}

	if(SetupNextViewMode())
	{
		RequestScreenshot();
	}
	else
	{
		// Current view is processed, cache the data
		CurrFrameData.Views.Emplace(CurrViewData);

		SetupFirstViewMode();

		if(GotoNextCameraView())
		{
			// Start a new view data
			CurrViewData.Clear();
			CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());

			RequestScreenshot();
		}
		else
		{
			// Write vision frame data to the database
			WriteFrameData();
			
			if(SetupNextEpisodeFrame())
			{
				GotoFirstCameraView();

				CurrViewData.Clear();
				CurrViewData.Init(VirtualCameras[CurrVirtualCameraIdx]->GetId(), VirtualCameras[CurrVirtualCameraIdx]->GetClassName());

				CurrFrameData.Clear();
				CurrFrameData.Init(CurrTimestamp, Resolution);

				RequestScreenshot();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] Finished visual logger.."),
					*FString(__func__), __LINE__, GetWorld()->GetTimeSeconds());
				QuitEditor();
			}
		}
	}
	
}

// Goto the first episode frame
bool USLVisionLogger::SetupFirstEpisodeFrame()
{
	if(!Episode.SetupFirstFrame(CurrTimestamp, true, OrigToMaskClones, SkelOrigToMaskClones))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d First frame not available.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Goto next episode frame, return false if there are no other left
bool USLVisionLogger::SetupNextEpisodeFrame()
{
	if(!Episode.SetupNextFrame(CurrTimestamp, true, OrigToMaskClones, SkelOrigToMaskClones))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d No new frames.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Goto the first virtual camera view
bool USLVisionLogger::GotoFirstCameraView()
{
	CurrVirtualCameraIdx = 0;
	if(!VirtualCameras.IsValidIndex(CurrVirtualCameraIdx))
	{
		CurrVirtualCameraIdx = INDEX_NONE;
		return false;
	}
	
	GetWorld()->GetFirstPlayerController()->SetViewTarget(VirtualCameras[CurrVirtualCameraIdx]);
	return true;

}

// Goto next camera view, return false if there are no other
bool USLVisionLogger::GotoNextCameraView()
{
	CurrVirtualCameraIdx++;
	if(!VirtualCameras.IsValidIndex(CurrVirtualCameraIdx))
	{
		CurrVirtualCameraIdx = INDEX_NONE;
		return false;
	}

	GetWorld()->GetFirstPlayerController()->SetViewTarget(VirtualCameras[CurrVirtualCameraIdx]);
	return true;
}

// Setup first view mode (render type)
bool USLVisionLogger::SetupFirstViewMode()
{
	CurrViewModeIdx = 0;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		return false;
	}
	
	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Setup next view mode (render type), return false if there are no other left
bool USLVisionLogger::SetupNextViewMode()
{
	CurrViewModeIdx++;
	if(!ViewModes.IsValidIndex(CurrViewModeIdx))
	{
		CurrViewModeIdx = INDEX_NONE;
		return false;
	}

	ApplyViewMode(ViewModes[CurrViewModeIdx]);
	return true;
}

// Connect to the database
bool USLVisionLogger::Connect(const FString& DBName, const FString& CollName, const FString& ServerIp,
	uint16 ServerPort, bool bRemovePrevEntries)
{
#if SL_WITH_LIBMONGO_C
	// Required to initialize libmongoc's internals	
	mongoc_init();

	// Stores any error that might appear during the connection
	bson_error_t error;

	// Safely create a MongoDB URI object from the given string
	FString Uri = TEXT("mongodb://") + ServerIp + TEXT(":") + FString::FromInt(ServerPort);
	uri = mongoc_uri_new_with_error(TCHAR_TO_UTF8(*Uri), &error);
	if (!uri)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.:%s; [Uri=%s]"),
			*FString(__func__), __LINE__, *FString(error.message), *Uri);
		return false;
	}

	// Create a new client instance
	client = mongoc_client_new_from_uri(uri);
	if (!client)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create a mongo client.."), *FString(__func__), __LINE__);
		return false;
	}

	// Register the application name so we can track it in the profile logs on the server
	mongoc_client_set_appname(client, TCHAR_TO_UTF8(*("SLVIS_" + CollName)));
	
	// Get a handle on the database "db_name" and collection "coll_name"
	database = mongoc_client_get_database(client, TCHAR_TO_UTF8(*DBName));

	// Give a warning if the collection already exists or not
	if (!mongoc_database_has_collection(database, TCHAR_TO_UTF8(*CollName), &error))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Collection %s does not exist, abort.."),
			*FString(__func__), __LINE__, *CollName);
		return false;
	}
	collection = mongoc_database_get_collection(database, TCHAR_TO_UTF8(*CollName));

	// Double check that the server is alive. Ping the "admin" database
	bson_t* server_ping_cmd;
	server_ping_cmd = BCON_NEW("ping", BCON_INT32(1));
	if (!mongoc_client_command_simple(client, "admin", server_ping_cmd, NULL, NULL, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Check server err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		bson_destroy(server_ping_cmd);
		return false;
	}
	bson_destroy(server_ping_cmd);

	// Remove previously added vision data
	if(bRemovePrevEntries)
	{
		ClearPreviousEntries();
	}

	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Disconnect and clean db connection
void USLVisionLogger::Disconnect()
{
#if SL_WITH_LIBMONGO_C
	// Release handles and clean up mongoc
	if(uri)
	{
		mongoc_uri_destroy(uri);
	}
	if(client)
	{
		mongoc_client_destroy(client);
	}
	if(database)
	{
		mongoc_database_destroy(database);
	}
	if(collection)
	{
		mongoc_collection_destroy(collection);
	}
	mongoc_cleanup();
#endif //SL_WITH_LIBMONGO_C
}

// Get the episode data from the database
bool USLVisionLogger::LoadEpisode(float UpdateRate)
{
	float CurrTs = 0.f;
	float PrevTs = - BIG_NUMBER; // this to make sure the first entry is loaded every time
	
#if SL_WITH_LIBMONGO_C
	bson_error_t error;
	const bson_t *doc;
	mongoc_cursor_t *cursor;
	bson_t *pipeline;

	pipeline = BCON_NEW("pipeline", "[",
		"{",
			"$match",
			"{",
				"timestamp",
				"{",
					"$exists", BCON_BOOL(true),
				"}",
			"}",
		"}",
		"{",
			"$sort",
			"{",
				"timestamp", BCON_INT32(1),
			"}",
		"}",
		"{",
			"$project",
			"{",
				"_id", BCON_INT32(0),
				"timestamp", BCON_INT32(1),
				"entities", BCON_UTF8("$entities"),
				"skel_entities", BCON_UTF8("$skel_entities"),
			"}",
		"}",
	"]");

	cursor = mongoc_collection_aggregate(
		collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

	bson_iter_t doc_iter;

	// Store the changes from the previous frame until this one
	FSLVisionFrame Frame;
	
	while (mongoc_cursor_next(cursor, &doc))
	{
		if (bson_iter_init(&doc_iter, doc))
		{
			// Get the current timestamp
			if (bson_iter_find(&doc_iter, "timestamp"))
			{
				CurrTs = bson_iter_double(&doc_iter);
			}

			// Accumulate entity changes in the frame until the desired update rate is reached
			ReadFramEntityDataFromBsonIterator(&doc_iter, Frame.ActorPoses);

			// Accumulate skeletal entity changes in the frame until the desired update rate is reached
			ReadFrameSkeletalEntityDataFromBsonIterator(&doc_iter, Frame.SkeletalPoses);

			// Check if the desired update rate is reached
			if(CurrTs - PrevTs >= UpdateRate)
			{
				// Update the previous timestamp
				PrevTs = CurrTs;

				// Add frame to episode and clear it for new data
				if(Frame.ActorPoses.Num() != 0 || Frame.SkeletalPoses.Num() != 0)
				{
					Frame.Timestamp = CurrTs;
					Episode.AddFrame(Frame);
					Frame.Clear();
				}
			}
		}
	}
	
	// Check if any errors appeared while iterating the cursor
	if (mongoc_cursor_error(cursor, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to iterate all documents.. Err. %s"),
			*FString(__func__), __LINE__, *FString(error.message));
		return false;
	}

	mongoc_cursor_destroy(cursor);
	bson_destroy(pipeline);
	
	return true;
#else
	return false;
#endif //SL_WITH_LIBMONGO_C
}

// Create movable clones of the skeletal meshes, hide originals (call before loading the episode data)
void USLVisionLogger::CreatePoseableMeshes()
{
	TArray<ASkeletalMeshActor*> SkeletalActors;
	FSLEntitiesManager::GetInstance()->GetSkeletalMeshActors(SkeletalActors);
	for(const auto& SkMA : SkeletalActors)
	{
		// Create a custom actor with a poseable mesh component
		FActorSpawnParameters SpawnParams;
		const FString LabelName = FString(TEXT("PMA_")).Append(SkMA->GetName());
		SpawnParams.Name = FName(*LabelName);
		ASLVisionPoseableMeshActor* PMA = GetWorld()->SpawnActor<ASLVisionPoseableMeshActor>(
			ASLVisionPoseableMeshActor::StaticClass(), SpawnParams);
		PMA->SetActorLabel(LabelName);

		if(PMA->Init(SkMA))
		{
			// Add actor to the quick access map
			SkelToPoseableMap.Emplace(SkMA, PMA);
		}

		// Hide original actor
		SkMA->SetActorHiddenInGame(true);
	}
}

// Load the pointers to the virtual cameras
bool USLVisionLogger::LoadVirtualCameras()
{
	FSLEntitiesManager::GetInstance()->GetCameraViewsObjects(VirtualCameras);
	return VirtualCameras.Num() > 0;
}

// Disable all actors physics and set them to movable
void USLVisionLogger::InitWorldEntities()
{
	// Disable physics on all actors
	for (TActorIterator<AActor> Act(GetWorld()); Act; ++Act)
	{
		Act->DisableComponentsSimulatePhysics();
		if(Act->GetRootComponent())
		{
			Act->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			// TODO check if static lights should be skipped
			Act->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no root component"), *FString(__func__), __LINE__, *Act->GetName());
		}
	}
}

// Create clones of the items with mask material on top
bool USLVisionLogger::CreateMaskClones()
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

	/* Static meshes */
	TArray<AStaticMeshActor*> SMActors;
	FSLEntitiesManager::GetInstance()->GetStaticMeshActors(SMActors);
	for(const auto& SMA : SMActors)
	{
		if(UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
		{
			// Get the mask color, will be black if it does not exist
			FColor SemColor(FColor::FromHex(FTags::GetValue(SMA, "SemLog", "VisMask")));
			if(SemColor == FColor::Black)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no visual mask, setting to black.."), *FString(__func__), __LINE__, *SMA->GetName());
			}
			
			// Create the mask material with the color
			UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
			DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));

			// Create the mask clone 
			FActorSpawnParameters Parameters;
			Parameters.Template = SMA;
			Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Parameters.Name = FName(*(SMA->GetName() + TEXT("_MaskClone")));
			AStaticMeshActor* SMAClone =  GetWorld()->SpawnActor<AStaticMeshActor>(SMA->GetClass(), Parameters);

			// Apply the generated mask material to the clone
			if(UStaticMeshComponent* CloneSMC = SMAClone->GetStaticMeshComponent())
			{
				for (int32 MatIdx = 0; MatIdx < CloneSMC->GetNumMaterials(); ++MatIdx)
				{
					CloneSMC->SetMaterial(MatIdx, DynamicMaskMaterial);
				}
			}

			// Hide and store the clone
			SMAClone->SetActorHiddenInGame(true);
			OrigToMaskClones.Emplace(SMA, SMAClone);
		}
	}

	/* Skeletal meshes */
	TArray<ASkeletalMeshActor*> SkMActors;
	FSLEntitiesManager::GetInstance()->GetSkeletalMeshActors(SkMActors);
	for(const auto& SkMA : SkMActors)
	{
		// Get the semantic data component containing the semantics (class names mask colors) about the bones
		if (UActorComponent* AC = SkMA->GetComponentByClass(USLSkeletalDataComponent::StaticClass()))
		{
			// Create a custom actor with a poseable mesh component
			FActorSpawnParameters SpawnParams;
			const FString LabelName = FString(TEXT("PMA_")).Append(SkMA->GetName()).Append("_MaskClone");
			SpawnParams.Name = FName(*LabelName);
			ASLVisionPoseableMeshActor* PMAClone = GetWorld()->SpawnActor<ASLVisionPoseableMeshActor>(
				ASLVisionPoseableMeshActor::StaticClass(), SpawnParams);
			PMAClone->SetActorLabel(LabelName);

			if (!PMAClone->Init(SkMA))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init the poseable mesh actor %s.."),
					*FString(__func__), __LINE__, *PMAClone->GetName());
				PMAClone->Destroy();
				continue;
			}

			// Apply mask materials
			USLSkeletalDataComponent* SkDC = CastChecked<USLSkeletalDataComponent>(AC);
			for (auto& Pair : SkDC->SemanticBonesData)
			{
				// Check if bone class and visual mask is set
				if(Pair.Value.IsSet())
				{
					// Get the mask color, will be black if it does not exist
					FColor SemColor(FColor::FromHex(Pair.Value.VisualMask));
					if (SemColor == FColor::Black)
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d %s --> %s has no visual mask, setting to black.."),
							*FString(__func__), __LINE__, *SkMA->GetName(), *Pair.Value.Class);
					}

					// Create the mask material with the color
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
						FLinearColor::FromSRGBColor(FColor::FromHex(Pair.Value.VisualMask)));

					PMAClone->SetCustomMaterial(Pair.Value.MaskMaterialIndex, DynamicMaskMaterial);
				}
			}

			// Hide and store the skeletal clone
			PMAClone->SetActorHiddenInGame(true);
			if(ASLVisionPoseableMeshActor** PMAOrig = SkelToPoseableMap.Find(SkMA))
			{
				SkelOrigToMaskClones.Add(*PMAOrig, PMAClone);
			}
			else
			{
				PMAClone->Destroy();
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no poseable mesh conterpart, mask clone will not be used.."),
					*FString(__func__), __LINE__, *SkMA->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Skeletal actor %s has no semantic data component, skipping.."),
				*FString(__func__), __LINE__, *SkMA->GetName());
		}
	}
	return true;
}

// Init hi-res screenshot resolution
void USLVisionLogger::InitScreenshotResolution(FIntPoint Resolution)
{
	// Set screenshot image and viewport resolution size
	GetHighResScreenshotConfig().SetResolution(Resolution.X, Resolution.Y, 1.0f);
	// Avoid triggering the callback be overwriting the resolution -> SetResolution() sets GIsHighResScreenshot to true, which triggers the callback (ScreenshotCB)
	GIsHighResScreenshot = false;	
}

// Init render parameters (resolution, view mode)
void USLVisionLogger::InitRenderParameters()
{
	// Defines the memory layout used for the GBuffer,
	// 0: lower precision (8bit per component, for profiling), 1: low precision (default)
	// 3: high precision normals encoding, 5: high precision
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.GBufferFormat"))->Set(5);

	
	// Set the near clipping plane (in cm)
	//IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetNearClipPlane"))->Set(0); // Not a console variable, but a command
	//GNearClippingPlane = 0; // View is distorted after finishing the scanning
#if WITH_EDITOR
	if(GEngine)
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

// Apply view mode
void USLVisionLogger::ApplyViewMode(ESLVisionLoggerViewMode Mode)
{
	// No change in the rendering view mode
	if(Mode == PrevViewMode)
	{
		return;
	}

	// Get the console variable for switching buffer views
	static IConsoleVariable* BufferVisTargetCV = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationTarget"));

	if(Mode == ESLVisionLoggerViewMode::Color) // viewmode=lit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionLoggerViewMode::Depth || PrevViewMode == ESLVisionLoggerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
		}
		CurrViewModePostfix = "C";
	}
	else if(Mode == ESLVisionLoggerViewMode::Unlit) // viewmode=unlit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionLoggerViewMode::Color)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Mask)
		{
			ApplyOriginalMaterials();
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Depth || PrevViewMode == ESLVisionLoggerViewMode::Normal)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		CurrViewModePostfix = "U";
	}
	else if(Mode == ESLVisionLoggerViewMode::Mask) // viewmode=unlit, buffer=false, materials=mask;
	{
		if(PrevViewMode == ESLVisionLoggerViewMode::Unlit)
		{
			ApplyMaskMaterials();
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Color)
		{
			ApplyMaskMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Depth || PrevViewMode == ESLVisionLoggerViewMode::Normal)
		{
			ApplyMaskMaterials();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		else
		{
			ApplyMaskMaterials();
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(false);
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode unlit");
		}
		CurrViewModePostfix = "M";
	}
	else if(Mode == ESLVisionLoggerViewMode::Depth) // viewmode=unlit, buffer=false, materials=original;
	{
		if(PrevViewMode == ESLVisionLoggerViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Normal)
		{
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		else
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("SLSceneDepthToCameraPlane"));
		}
		CurrViewModePostfix = "D";
	}
	else if(Mode == ESLVisionLoggerViewMode::Normal) // viewmode=lit, buffer=true, materials=original;
	{
		if(PrevViewMode == ESLVisionLoggerViewMode::Depth)
		{
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Mask)
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Unlit)
		{
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else if(PrevViewMode == ESLVisionLoggerViewMode::Color)
		{
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		else
		{
			ApplyOriginalMaterials();
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("viewmode lit");
			ViewportClient->GetEngineShowFlags()->SetVisualizeBuffer(true);
			BufferVisTargetCV->Set(*FString("WorldNormal"));
		}
		CurrViewModePostfix = "N";
	}

	// Cache as previous view mode
	PrevViewMode = Mode;
}

// Apply mask materials 
void USLVisionLogger::ApplyMaskMaterials()
{
	for(const auto& Pair : OrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(true);
		Pair.Value->SetActorHiddenInGame(false);
	}
	for (const auto& Pair : SkelOrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(true);
		Pair.Value->SetActorHiddenInGame(false);
	}
}

// Apply original material to current item
void USLVisionLogger::ApplyOriginalMaterials()
{
	for(const auto& Pair : OrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(false);
		Pair.Value->SetActorHiddenInGame(true);
	}
	for (const auto& Pair : SkelOrigToMaskClones)
	{
		Pair.Key->SetActorHiddenInGame(false);
		Pair.Value->SetActorHiddenInGame(true);
	}
}

// Write the current vision frame data to the database
void USLVisionLogger::WriteFrameData() const
{
#if SL_WITH_LIBMONGO_C
	// Document holding the frame data in bson format
	bson_t frame_doc;
	bson_init(&frame_doc);

	// Add resolution sub doc
	bson_t res_sub_doc;

	BSON_APPEND_DOCUMENT_BEGIN(&frame_doc, "res", &res_sub_doc);
	BSON_APPEND_INT32(&res_sub_doc, "x", CurrFrameData.Resolution.X);
	BSON_APPEND_INT32(&res_sub_doc, "y", CurrFrameData.Resolution.Y);
	bson_append_document_end(&frame_doc, &res_sub_doc);

	bson_t views_arr;
	bson_t views_arr_obj;

	bson_t entities_arr;
	bson_t entities_arr_obj;

	//bson_t skel_arr;
	//bson_t skel_arr_obj;

	bson_t bones_arr;
	bson_t bones_arr_obj;

	char i_str[16];
	const char *i_key;
	uint32_t i = 0;

	char j_str[16];
	const char *j_key;
	uint32_t j = 0;

	char k_str[16];
	const char *k_key;
	uint32_t k = 0;

	// Begin adding views data tot the 
	BSON_APPEND_ARRAY_BEGIN(&frame_doc, "views", &views_arr);
	
	// Iterate views (virtual cameras)
	for (const auto& ViewData : CurrFrameData.Views)
	{
		// Start array entry
		bson_uint32_to_string(i, &i_key, i_str, sizeof i_str);
		BSON_APPEND_DOCUMENT_BEGIN(&views_arr, i_key, &views_arr_obj);

			BSON_APPEND_UTF8(&views_arr_obj, "class", TCHAR_TO_UTF8(*ViewData.Class));
			BSON_APPEND_UTF8(&views_arr_obj, "id", TCHAR_TO_UTF8(*ViewData.Id));

			// Create the entities array
			j = 0;
			BSON_APPEND_ARRAY_BEGIN(&views_arr_obj, "entities", &entities_arr);
			for (const auto& Entity : ViewData.Entities)
			{
				bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
				BSON_APPEND_DOCUMENT_BEGIN(&entities_arr, j_key, &entities_arr_obj);

					BSON_APPEND_UTF8(&entities_arr_obj, "id", TCHAR_TO_UTF8(*Entity.Id));
					BSON_APPEND_UTF8(&entities_arr_obj, "class", TCHAR_TO_UTF8(*Entity.Class));

				bson_append_document_end(&entities_arr, &entities_arr_obj);
				j++;
			}
			bson_append_array_end(&views_arr_obj, &entities_arr);

			// Create the skeletal entities array
			j = 0;
			BSON_APPEND_ARRAY_BEGIN(&views_arr_obj, "skel_entities", &entities_arr);
			for (const auto& SkelEntity : ViewData.SkelEntities)
			{
				bson_uint32_to_string(j, &j_key, j_str, sizeof j_str);
				BSON_APPEND_DOCUMENT_BEGIN(&entities_arr, j_key, &entities_arr_obj);

					BSON_APPEND_UTF8(&entities_arr_obj, "id", TCHAR_TO_UTF8(*SkelEntity.Id));
					BSON_APPEND_UTF8(&entities_arr_obj, "class", TCHAR_TO_UTF8(*SkelEntity.Class));

					// Create the bones array
					k = 0;
					BSON_APPEND_ARRAY_BEGIN(&entities_arr_obj, "bones", &bones_arr);
					for (const auto& Bone : SkelEntity.Bones)
					{
						bson_uint32_to_string(k, &k_key, k_str, sizeof k_str);
						BSON_APPEND_DOCUMENT_BEGIN(&bones_arr, k_key, &bones_arr_obj);

							BSON_APPEND_UTF8(&bones_arr_obj, "class", TCHAR_TO_UTF8(*Bone.Class));

						bson_append_document_end(&bones_arr, &bones_arr_obj);
						k++;
					}
					bson_append_array_end(&entities_arr_obj, &bones_arr);

				bson_append_document_end(&entities_arr, &entities_arr_obj);
				j++;
			}
			bson_append_array_end(&views_arr_obj, &entities_arr);


		// End array entry
		bson_append_document_end(&views_arr, &views_arr_obj);
		i++;
	}
	bson_append_array_end(&frame_doc, &views_arr);

	// Update DB at the given timestamp with the document
	UpdateDBWithFrameData(&frame_doc, CurrFrameData.Timestamp);

	bson_destroy(&frame_doc);
#endif //SL_WITH_LIBMONGO_C
}

// Remove any previously added vision data from the database
void USLVisionLogger::ClearPreviousEntries() const
{
#if SL_WITH_LIBMONGO_C
	// Remove all previous entries
	bson_t *selector = BCON_NEW("vision", "{", "$exists", BCON_BOOL(true), "}");
	bson_t *update = BCON_NEW("$unset", "{", "vision", BCON_BOOL(true), "}");
	bson_t reply;
	bson_error_t error;
	if (!mongoc_collection_update_many(collection, selector, update, NULL, &reply, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Check the number removed entries
	bson_iter_t iter;
	if (bson_iter_init_find(&iter, &reply, "modifiedCount") && BSON_ITER_HOLDS_INT(&iter))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Number of removed entries = %d"),
			*FString(__func__), __LINE__, bson_iter_int32(&iter));
	}

	// Drop previous indexes to allow fast inserts
	if (!mongoc_collection_drop_index(collection, "vision_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.id_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.class_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.entities.id_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.entities.class_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.skel_entities.id_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.skel_entities.class_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}
	if (!mongoc_collection_drop_index(collection, "vision.views.skel_entities.bones.class_1", &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	bson_destroy(selector);
	bson_destroy(update);
	bson_destroy(&reply);
#endif //SL_WITH_LIBMONGO_C
}

// Clean exit, all the Finish() methods will be triggered
void USLVisionLogger::QuitEditor()
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

// Save the compressed screenshot image locally
void USLVisionLogger::SaveImageLocally(const TArray<uint8>& CompressedBitmap)
{
	const FString FolderName = VirtualCameras[CurrVirtualCameraIdx]->GetClassName() + "_" + CurrViewModePostfix;
	FString Path = FPaths::ProjectDir() + "/SemLog/" + TaskId + "/" + FolderName + "/" + CurrImageFilename + ".png";
	FPaths::RemoveDuplicateSlashes(Path);
	FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
}

// Output progress to terminal
void USLVisionLogger::PrintProgress() const
{
	const int32 CurrFrameNr = Episode.GetCurrIndex() + 1;
	const int32 TotalFrames = Episode.GetFramesNum();
	const int32 CurrCameraNr = CurrVirtualCameraIdx + 1;
	const int32 TotalCameras = VirtualCameras.Num();
	const int32 CurrViewModeNr = CurrViewModeIdx + 1;
	const int32 TotalViewModes = ViewModes.Num();
	const float LastTs = Episode.GetLastTimestamp();
	const int32 CurrImgNr = Episode.GetCurrIndex() * TotalCameras * TotalViewModes + CurrVirtualCameraIdx * TotalViewModes + CurrViewModeNr;
	const int32 TotalImgs = TotalFrames * TotalCameras * TotalViewModes;

	UE_LOG(LogTemp, Log, TEXT("%s::%d \t\t Frame=%ld/%ld; \t\t Camera=%ld/%ld; \t\t ViewMode=%ld/%ld; \t\t Image=%ld/%ld; \t\t Ts=%f/%f;"),
		*FString(__func__), __LINE__,
		CurrFrameNr, TotalFrames,
		CurrCameraNr, TotalCameras,
		CurrViewModeNr, TotalViewModes,
		CurrImgNr, TotalImgs,
		CurrTimestamp, LastTs);
}

// Create indexes on the inserted data
void USLVisionLogger::CreateIndexes() const
{
#if SL_WITH_LIBMONGO_C
	bson_t* index_command;
	bson_error_t error;

	bson_t index;
	bson_init(&index);
	BSON_APPEND_INT32(&index, "vision", 1);
	char* index_str = mongoc_collection_keys_to_index_string(&index);

	bson_t idx_id;
	bson_init(&idx_id);
	BSON_APPEND_INT32(&idx_id, "vision.views.id", 1);
	char* idx_id_str = mongoc_collection_keys_to_index_string(&idx_id);

	bson_t idx_cls;
	bson_init(&idx_cls);
	BSON_APPEND_INT32(&idx_cls, "vision.views.class", 1);
	char* idx_cls_str = mongoc_collection_keys_to_index_string(&idx_cls);

	bson_t idx_eid;
	bson_init(&idx_eid);
	BSON_APPEND_INT32(&idx_eid, "vision.views.entities.id", 1);
	char* idx_eid_str = mongoc_collection_keys_to_index_string(&idx_eid);

	bson_t idx_ecls;
	bson_init(&idx_ecls);
	BSON_APPEND_INT32(&idx_ecls, "vision.views.entities.class", 1);
	char* idx_ecls_str = mongoc_collection_keys_to_index_string(&idx_ecls);

	bson_t idx_skeid;
	bson_init(&idx_skeid);
	BSON_APPEND_INT32(&idx_skeid, "vision.views.skel_entities.id", 1);
	char* idx_skeid_str = mongoc_collection_keys_to_index_string(&idx_skeid);

	bson_t idx_skecls;
	bson_init(&idx_skecls);
	BSON_APPEND_INT32(&idx_skecls, "vision.views.skel_entities.class", 1);
	char* idx_skecls_str = mongoc_collection_keys_to_index_string(&idx_skecls);

	bson_t idx_skebcls;
	bson_init(&idx_skebcls);
	BSON_APPEND_INT32(&idx_skebcls, "vision.views.skel_entities.bones.class", 1);
	char* idx_skebcls_str = mongoc_collection_keys_to_index_string(&idx_skebcls);

	index_command = BCON_NEW("createIndexes",
		BCON_UTF8(mongoc_collection_get_name(collection)),
		"indexes",
		"[",
			"{",
				"key",
				BCON_DOCUMENT(&index),
				"name",
				BCON_UTF8(index_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_id),
				"name",
				BCON_UTF8(idx_id_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_cls),
				"name",
				BCON_UTF8(idx_cls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_eid),
				"name",
				BCON_UTF8(idx_eid_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_ecls),
				"name",
				BCON_UTF8(idx_ecls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skeid),
				"name",
				BCON_UTF8(idx_skeid_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skecls),
				"name",
				BCON_UTF8(idx_skecls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
			"{",
				"key",
				BCON_DOCUMENT(&idx_skebcls),
				"name",
				BCON_UTF8(idx_skebcls_str),
				//"unique",
				//BCON_BOOL(true),
			"}",
		"]");

	if (!mongoc_collection_write_command_with_opts(collection, index_command, NULL/*opts*/, NULL/*reply*/, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Create indexes err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Clean up
	bson_destroy(index_command);
	bson_free(index_str);
	bson_free(idx_id_str);
	bson_free(idx_cls_str);
#endif //SL_WITH_LIBMONGO_C
}

#if SL_WITH_LIBMONGO_C
// Get the entities data out of the bson iterator
bool USLVisionLogger::ReadFramEntityDataFromBsonIterator(bson_iter_t* doc, TMap<AActor*, FTransform>& OutEntityPoses) const
{
	// Iterate entities
	if (bson_iter_find(doc, "entities"))
	{
		bson_iter_t child_iter;				// entities,
		bson_iter_t sub_child_iter;			// id, loc, rot
		bson_iter_t sub_sub_child_iter;		// x,y,z,w (entity)
		
		// Check if there are any entities
		if (bson_iter_recurse(doc, &child_iter))
		{
			FString Id;
			FVector Loc;
			FQuat Quat;
					
			while(bson_iter_next(&child_iter))
			{
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find(&sub_child_iter, "id"))
				{
					Id = FString(bson_iter_utf8(&sub_child_iter, NULL));
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.x", &sub_sub_child_iter))
				{
					Loc.X = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.y", &sub_sub_child_iter))
				{
					Loc.Y = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "loc.z", &sub_sub_child_iter))
				{
					Loc.Z = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.x", &sub_sub_child_iter))
				{
					Quat.X = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.y", &sub_sub_child_iter))
				{
					Quat.Y = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.z", &sub_sub_child_iter))
				{
					Quat.Z = bson_iter_double(&sub_sub_child_iter);
				}
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find_descendant(&sub_child_iter, "rot.w", &sub_sub_child_iter))
				{
					Quat.W = bson_iter_double(&sub_sub_child_iter);
				}

				// Add entity
				if(AActor* Act = FSLEntitiesManager::GetInstance()->GetActor(Id))
				{
					OutEntityPoses.Emplace(Act, FConversions::ROSToU(FTransform(Quat, Loc)));
				}
			}
		}
		return OutEntityPoses.Num() > 0;
	}
	else
	{
		return false;
	}
}

// Get the entities data out of the bson iterator, returns false if there are no entities
bool USLVisionLogger::ReadFrameSkeletalEntityDataFromBsonIterator(bson_iter_t* doc, TMap<ASLVisionPoseableMeshActor*, TMap<FName, FTransform>>& OutSkeletalPoses) const
{
	// Iterate skeletal entities
	if (bson_iter_find(doc, "skel_entities"))
	{
		bson_iter_t child_iter;				// skel_entities
		bson_iter_t sub_child_iter;			// bones
		bson_iter_t sub_sub_child_iter;		// bones (array)
		
		if (bson_iter_recurse(doc, &child_iter))
		{
			FString Id;
			TMap<FName, FTransform> BonesMap;
					
			while (bson_iter_next(&child_iter))
			{
				if (bson_iter_recurse(&child_iter, &sub_child_iter) && bson_iter_find(&sub_child_iter, "id"))
				{
					Id = FString(bson_iter_utf8(&sub_child_iter, NULL));
				}
						
				if (bson_iter_recurse(&child_iter, &sub_sub_child_iter) && bson_iter_find(&sub_sub_child_iter, "bones"))
				{
					bson_iter_t bones_child;			// array  obj
					bson_iter_t bones_sub_child;		// name, loc, rot
					bson_iter_t bones_sub_sub_child;	// x, y , z, w
							
					FName BoneName;
					FVector Loc;
					FQuat Quat;
							
					if (bson_iter_recurse(&sub_sub_child_iter, &bones_child))
					{
						while (bson_iter_next(&bones_child))
						{
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find(&bones_sub_child, "name"))
							{
								BoneName = FName(bson_iter_utf8(&bones_sub_child, NULL));
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.x", &bones_sub_sub_child))
							{
								Loc.X = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.y", &bones_sub_sub_child))
							{
								Loc.Y = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "loc.z", &bones_sub_sub_child))
							{
								Loc.Z = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.x", &bones_sub_sub_child))
							{
								Quat.X = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.y", &bones_sub_sub_child))
							{
								Quat.Y = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.z", &bones_sub_sub_child))
							{
								Quat.Z = bson_iter_double(&bones_sub_sub_child);
							}
							if (bson_iter_recurse(&bones_child, &bones_sub_child) && bson_iter_find_descendant(&bones_sub_child, "rot.w", &bones_sub_sub_child))
							{
								Quat.W = bson_iter_double(&bones_sub_sub_child);
							}
									
							BonesMap.Add(BoneName, FConversions::ROSToU(FTransform(Quat, Loc)));
						}
					}
				}

				// Add skeletal entity
				if(ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor((Id)))
				{
					if(ASLVisionPoseableMeshActor* const* PMA = SkelToPoseableMap.Find(SkMA))
					{
						OutSkeletalPoses.Emplace(*PMA, BonesMap);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find poseable mesh clone actor for %s, did you run the setup before?"),
							*FString(__func__), __LINE__, *SkMA->GetName());
					}
				}
			}
		}
		return OutSkeletalPoses.Num() > 0;
	}
	return false;
}

// Write the bson doc containing the vision data to the entry corresponding to the timestamp
bool USLVisionLogger::UpdateDBWithFrameData(bson_t* doc, float Timestamp) const
{
	// Return flag
	bool bSuccess = true;

	// Update the entry at the given timestamp
	bson_t* selector = BCON_NEW("timestamp", BCON_DOUBLE(CurrFrameData.Timestamp));
	bson_t* update = BCON_NEW("$set", "{", "vision", BCON_DOCUMENT(doc), "}");
	bson_t reply;
	bson_error_t error;
	if (!mongoc_collection_update_one(collection, selector, update, NULL, &reply, &error))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Err.: %s"),
			*FString(__func__), __LINE__, *FString(error.message));
	}

	// Check if the entry was updated
	bson_iter_t iter;
	if (bson_iter_init_find(&iter, &reply, "modifiedCount") && BSON_ITER_HOLDS_INT(&iter))
	{
		int32 NumOfUpdatedEntries = bson_iter_int32(&iter);
		if (NumOfUpdatedEntries == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not update entry at timestamp %f"),
				*FString(__func__), __LINE__, CurrFrameData.Timestamp);
			bSuccess = false;
		}
		else if (NumOfUpdatedEntries > 1)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Updated more than one (%d) entry at timestamp %f, this should not happen.."),
				*FString(__func__), __LINE__, NumOfUpdatedEntries, CurrFrameData.Timestamp);
			bSuccess = false;
		}
		else if (NumOfUpdatedEntries == 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Succesfully updated entry at timestamp %f"),
				*FString(__func__), __LINE__, CurrFrameData.Timestamp);
			bSuccess = true;
		}
	}

	// Cleanup
	bson_destroy(selector);
	bson_destroy(update);
	bson_destroy(&reply);

	return bSuccess;
}
#endif //SL_WITH_LIBMONGO_C
