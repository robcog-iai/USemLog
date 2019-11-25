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
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"

// UUtils
#include "Conversions.h"

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	ViewModes.Add(ESLVisionLoggerViewMode::Color);
	ViewModes.Add(ESLVisionLoggerViewMode::Unlit);
	ViewModes.Add(ESLVisionLoggerViewMode::Mask);
	ViewModes.Add(ESLVisionLoggerViewMode::Depth);
	ViewModes.Add(ESLVisionLoggerViewMode::Normal);
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
	const FSLVisionLoggerParams& Params)
{
	if (!bIsInit)
	{
		// Save the folder name if the images are going to be stored locally as well
		if(Params.bIncludeLocally)
		{
			ImgsFolderName = InTaskId;
		}
		
		// Init the semantic instances
		FSLEntitiesManager::GetInstance()->Init(GetWorld());
		
		// Make sure the poseable mesh actors are created before
		SetupPoseableMeshes();

		// Connect to the database
		if(!Connect(InTaskId, InEpisodeId, InServerIp, InServerPort))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not connect to the DB.."), *FString(__func__), __LINE__);
			return;
		}

		// Load the episode data (the poseable meshes should be setup before)
		if(!GetEpisodeData())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could download the episode data.."), *FString(__func__), __LINE__);
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

		// Dynamic material used to add the mask colors
		if(!LoadMaskMaterial())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load the mask material.."), *FString(__func__), __LINE__);
			return;
		}
		
		// Create clones of every visible entity with the mask color
		SetupMaskClones();

		// Set the captured image resolution
		InitScreenshotResolution(Params.Resolution);

		// Set rendering parameters
		InitRenderParameters();

		// Disable physics on all entities and make sure they are movable
		SetupWorldEntities();

		// Bind the screenshot callback
		ViewportClient->OnScreenshotCaptured().AddUObject(this, &USLVisionLogger::ScreenshotCB);
		
		bIsInit = true;
	}
}

// Start logger
void USLVisionLogger::Start(const FString& EpisodeId)
{
	if (!bIsStarted && bIsInit)
	{
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
		if(!SetupFirstEpisodeFrame())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup the first world frame.."), *FString(__func__), __LINE__);
			return;
		}
		
		// Cannot be called before BeginPlay, GetFirstPlayerController() is nullptr
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
		
		// FrameNum_CameraNum_Ts_Viewmode
		CurrImageFilename = FString::FromInt(Episode.GetCurrIndex()) + "_" + 
			FString::FromInt(CurrVirtualCameraIdx) + "_" + 
			FString::SanitizeFloat(CurrTimestamp) + "_" + CurrViewModePostfix;
		
		GetHighResScreenshotConfig().FilenameOverride = CurrImageFilename;
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		ViewportClient->Viewport->TakeHighResScreenShot();
	});
}


// Called when the screenshot is captured
void USLVisionLogger::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
	PrintProgress();

	// Compress image
	TArray<uint8> CompressedBitmap;
	FImageUtils::CompressImageArray(SizeX, SizeY, Bitmap, CompressedBitmap);

	// Save the png locally
	if(!ImgsFolderName.IsEmpty())
	{
		FString Path = FPaths::ProjectDir() + "/SemLog/" + ImgsFolderName + "/" + CurrImageFilename + ".png";
		FPaths::RemoveDuplicateSlashes(Path);
		FFileHelper::SaveArrayToFile(CompressedBitmap, *Path);
	}

	if(SetupNextViewMode())
	{
		RequestScreenshot();
	}
	else
	{
		SetupFirstViewMode();

		if(GotoNextCameraView())
		{
			RequestScreenshot();
		}
		else
		{
			GotoFirstCameraView();
			
			if(SetupNextEpisodeFrame())
			{
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
	if(!Episode.SetupFirstFrame(CurrTimestamp))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup the episode first frame.."), *FString(__func__), __LINE__);
		return false;
	}
	return true;
}

// Goto next episode frame, return false if there are no other left
bool USLVisionLogger::SetupNextEpisodeFrame()
{
	if(Episode.SetupNextFrame(CurrTimestamp))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not setup the episode next frame.."), *FString(__func__), __LINE__);
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
	uint16 ServerPort)
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
bool USLVisionLogger::GetEpisodeData()
{
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
	bson_iter_t child_iter;				// entities, skel_entities
	bson_iter_t sub_child_iter;			// id, bones, loc, rot
	bson_iter_t sub_sub_child_iter;		// x,y,z,w (entity), bones (array)
	
	while (mongoc_cursor_next(cursor, &doc))
	{
		FSLVisionFrame Frame;

		if (bson_iter_init(&doc_iter, doc))
		{
			// Iterate entities
			if (bson_iter_find(&doc_iter, "entities"))
			{
				// Check if there are any entities
				if (bson_iter_recurse(&doc_iter, &child_iter))
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
							Frame.ActorPoses.Emplace(Act, FConversions::ROSToU(FTransform(Quat, Loc)));
						}
					}
				}
			}

			// Iterate skeletal entities
			if (bson_iter_find(&doc_iter, "skel_entities"))
			{
				if (bson_iter_recurse(&doc_iter, &child_iter))
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
							if(ASLVisionPoseableMeshActor** PMA = SkMAToPMA.Find(SkMA))
							{
								Frame.PoseableMeshPoses.Emplace(*PMA, BonesMap);
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find poseable mesh clone actor for %s, did you run the setup before?"),
									*FString(__func__), __LINE__, *SkMA->GetName());
							}
						}
					}
				}
			}
		}

		// Add to episode only if there are any changes
		if(Frame.ActorPoses.Num() != 0 || Frame.PoseableMeshPoses.Num() != 0)
		{
			if (bson_iter_find(&doc_iter, "timestamp"))
			{
				Frame.Timestamp = bson_iter_double(&doc_iter);
			}
			Episode.AddFrame(Frame);
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

// Setup the poseable mesh clones for the skeletal ones
void USLVisionLogger::SetupPoseableMeshes()
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
			SkMAToPMA.Emplace(SkMA, PMA);
		}

		// Hide orginal actor
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
void USLVisionLogger::SetupWorldEntities()
{
	// Disable physics on all actors
	for (TActorIterator<AActor> Act(GetWorld()); Act; ++Act)
	{
		Act->DisableComponentsSimulatePhysics();
		if(Act->GetRootComponent())
		{
			// TODO check if lights should be skipped
			Act->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no root component"), *FString(__func__), __LINE__, *Act->GetName());
		}
	}
}

// Load mask dynamic material
bool USLVisionLogger::LoadMaskMaterial()
{
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this,
	TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material.."),
			*FString(__func__), __LINE__);
		return false;
	}
	
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create the dynamic mask material from the default one
	DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
	DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::White);
	return true;
}

// Create clones of the items with mask material on top
bool USLVisionLogger::SetupMaskClones()
{
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
}

// Apply original material to current item
void USLVisionLogger::ApplyOriginalMaterials()
{
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

// Output progress to terminal
void USLVisionLogger::PrintProgress() const
{
	const int32 CurrFrameNr = Episode.GetCurrIndex() + 1;
	const int32 TotalFrames = Episode.GetFramesNum();
	const int32 CurrCameraNr = CurrVirtualCameraIdx + 1;
	const int32 TotalCameras = VirtualCameras.Num();
	const int32 CurrViewModeNr = CurrViewModeIdx + 1;
	const int32 TotalViewModes = ViewModes.Num();
	UE_LOG(LogTemp, Log, TEXT("%s::%d \t\t [%f] \t\t Progress= \t\t Frame=%ld/%ld; \t\t Camera=%ld/%ld; \t\t ViewMode=%ld/%ld; \t\t Image=%ld/%ld;"),
		*FString(__func__), __LINE__, CurrTimestamp,
		CurrFrameNr, TotalFrames,
		CurrCameraNr, TotalCameras,
		CurrViewModeNr, TotalViewModes,
		CurrFrameNr * TotalFrames * TotalViewModes + CurrCameraNr * TotalViewModes + CurrViewModeNr,
		TotalFrames * TotalCameras * TotalViewModes);
}