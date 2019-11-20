// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionLogger.h"
#include "SLEntitiesManager.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	ViewModes.Add(ESLVisLoggerViewMode::Color);
	ViewModes.Add(ESLVisLoggerViewMode::Unlit);
	ViewModes.Add(ESLVisLoggerViewMode::Mask);
	ViewModes.Add(ESLVisLoggerViewMode::Depth);
	ViewModes.Add(ESLVisLoggerViewMode::Normal);
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
		if(!Connect(InTaskId, InEpisodeId, InServerIp, InServerPort))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not connect to the DB.."), *FString(__func__), __LINE__);
			return;
		}

		// If no view modes are available, add a default one
		if(ViewModes.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d No view modes found.."), *FString(__func__), __LINE__);
			return;
		}

		// Used for the screenshot requests
		ViewportClient = GetWorld()->GetGameViewport();
		if(!ViewportClient)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the GameViewport.."), *FString(__func__), __LINE__);
			return;
		}

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
		DisablePhysics();

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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLVisionLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d"), *FString(__func__), __LINE__);
		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Called when the screenshot is captured
void USLVisionLogger::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
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

// Disable physics and set to movable
void USLVisionLogger::DisablePhysics()
{
	// Disable physics on all actors
	for (TActorIterator<AActor> Act(GetWorld()); Act; ++Act)
	{
		Act->DisableComponentsSimulatePhysics();
		Act->GetRootComponent()->SetMobility(EComponentMobility::Movable); // TODO check if lights should be skipped
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
	if(GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("r.SetNearClipPlane 0"));
	}
	
	// AAM_None=None, AAM_FXAA=FXAA, AAM_TemporalAA=TemporalAA, AAM_MSAA=MSAA (Only supported with forward shading.  MSAA sample count is controlled by r.MSAACount)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"))->Set(AAM_None);

	// Whether the default for AutoExposure is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AutoExposure"))->Set(0);

	// Whether the default for MotionBlur is enabled or not (postprocess volume/camera/game setting can still override and enable or disable it independently)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.MotionBlur"))->Set(0);

	// LOD level to force, -1 is off. (0 - Best)
	IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForceLOD"))->Set(0);
}

// Request a screenshot
void USLVisionLogger::RequestScreenshot()
{
}

// Apply view mode
void USLVisionLogger::ApplyViewMode(ESLVisLoggerViewMode Mode)
{
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
