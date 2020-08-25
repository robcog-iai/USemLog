// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManager.h"
#include "EngineUtils.h"
#include "SLEntitiesManager.h"
#include "Ids.h"

// Sets default values
ASLManager::ASLManager()
{
	// Disable tick on actor
	PrimaryActorTick.bCanEverTick = false;

	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Semantic logger default values
	TaskId = TEXT("DefaultTaskId");
	bUseCustomEpisodeId = false;
	EpisodeId = TEXT("");
	TaskDescription = TEXT("Write task description here");
	bResetStartTime = false;
	bStartAtBeginPlay = true;
	bStartAtFirstTick = false;
	bStartWithDelay = false;
	StartDelay = 0.5f;
	bStartFromUserInput = false;
	UserInputActionName = "SLTrigger";

	// Server TaskId
	ServerIp = TEXT("127.0.0.1");
	ServerPort = 27017;
	
	// Task metadata logger default values
	bLogMetadata = false;
	bOverwriteMetadata = false;
	bScanItems = false;
	ScanResolution.X = 1080;
	ScanResolution.Y = 1080;
	NumberOfScanPoints = 64;
	MaxScanItemVolume = 0.f;
	CameraDistanceToScanItem = 0.f;
	bIncludeScansLocally = false;
	
	// World state logger default values
	bLogWorldState = true;
	bOverwriteWorldState = false;
	WorldStateUpdateRate = 0.0f;
	LinearDistance = 0.5f; // cm
	AngularDistance = 0.1f; // rad
	WriterType = ESLWorldWriterType::MongoC;

	
	// Events logger default values
	bLogEventData = true;
	bLogContactEvents = true;
	bLogSupportedByEvents = true;
	bLogGraspEvents = true;
	bLogPickAndPlaceEvents = true;
	bLogSlicingEvents = true;
	bWriteTimelines = true;
	bWriteEpisodeMetadata = false;
	ExperimentTemplateType = ESLOwlExperimentTemplate::Default;

	
	// Vision data logger default values
	bLogVisionData = false;
	bOverwriteVisionData = true;
	VisionUpdateRate = 0.f;
	VisionImageResolution = FIntPoint(1920, 1080);
	bCalculateOverlaps = true;
	OverlapResolutionDivisor = 4;
	bIncludeImagesLocally = false;

	// Editor Logger default values
	bLogEditorData = false;
	bOverwriteEditorData = false;
	bWriteSemanticMap = false;
	bClearAllTags = false;
	TagTypeToClear = "SemLog";
	TagKeyToClear = "";
	bWriteClassTags = false;
	bWriteUniqueIdTags = false;
	bWriteUniqueMaskColors = false;
	MinColorManhattanDistance = 17;
	bUseRandomColorGeneration = false;
	bWriteNonMovableTags = false;
	bCalibrateRenderedMaskColors = false;
	bMaskColorsOnlyDemo = false;
	EditorAssetAction = ESLAssetAction::NONE;

	// Data visualzer default values
	bVisualizeData = false;
	
#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.65;
#endif // WITH_EDITORONLY_DATA


	/****/
	ActualWorldStateLogger = nullptr;
}

// Sets default values
ASLManager::~ASLManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(-1.f, true);
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Init loggers
	Init();
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		Start();
	}
	else if (bStartAtFirstTick)
	{
		FTimerDelegate TimerDelegateNextTick;
		TimerDelegateNextTick.BindLambda([this]
		{
			Start();
		});
		GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
	}
	else if (bStartWithDelay)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegateDelay;
		TimerDelegateDelay.BindLambda([this]
		{
			Start();
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartDelay, false);
	}
	else if (bStartFromUserInput)
	{
		// Bind user input
		SetupInputBindings();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Start() will not be called.."), *FString(__func__), __LINE__);
	}
}

// Called when actor removed from game or game ended
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish(GetWorld()->GetTimeSeconds());
	}
}

// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
void ASLManager::PostLoad()
{
	Super::PostLoad();
	// Setup references
	
	//if (!ActualWorldStateLogger
	//	|| !ActualWorldStateLogger->IsValidLowLevel()
	//	|| ActualWorldStateLogger->IsPendingKillOrUnreachable()
	//	|| !ActualWorldStateLogger->CheckStillInWorld())
	//{
	//	// Search in the world
	//	for (TActorIterator<ASLWorldStateLogger> Iter(GetWorld()); Iter; ++Iter)
	//	{
	//		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
	//		{
	//			ActualWorldStateLogger = *Iter;
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Reference to the world state logger (%s) set.."),
	//				*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//			return;
	//		}
	//		else
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d Reference (%s) invalid.."),
	//				*FString(__FUNCTION__), __LINE__, *Iter->GetName());
	//		}
	//	}

	//	// Not found in the world, spawn new
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Name = TEXT("SL_WorldStateLogger");
	//	ActualWorldStateLogger = GetWorld()->SpawnActor<ASLWorldStateLogger>(SpawnParams);
	//	//ActualWorldStateLogger->SetActorLabel(TEXT("SL_WorldStateLogger"));

	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d World state logger not found in the world, spawned new one (%s).."),
	//		*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d ActualWorldStateLogger (%s) is set.."),
	//		*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//}
}

// Init loggers
void ASLManager::Init()
{
	if (!bIsInit)
	{
		// Init the semantic items content singleton
		FSLEntitiesManager::GetInstance()->Init(GetWorld());

		if (bLogMetadata)
		{
			MetadataLogger = NewObject<USLMetadataLogger>(this);
			MetadataLogger->Init(TaskId, ServerIp, ServerPort,
				bOverwriteMetadata, bScanItems, 
				FSLMetaScannerParams(ScanResolution, NumberOfScanPoints, MaxScanItemVolume, CameraDistanceToScanItem, bIncludeScansLocally));
		}
		else if(bLogEditorData)
		{
			EditorLogger = NewObject<USLEditorLogger>(this);
			EditorLogger->Init(TaskId, ServerIp, ServerPort, EditorAssetAction, bCalibrateRenderedMaskColors, bMaskColorsOnlyDemo, bOverwriteEditorData);
		}
		else if (bLogVisionData)
		{
			VisionDataLogger = NewObject<USLVisionLogger>(this);
			VisionDataLogger->Init(TaskId, EpisodeId, ServerIp, ServerPort, bOverwriteVisionData,
				FSLVisionLoggerParams(VisionUpdateRate, VisionImageResolution, bIncludeImagesLocally, bCalculateOverlaps, OverlapResolutionDivisor));
		}
		else if (bVisualizeData)
		{
			DataVisualizer = NewObject<USLDataVisualizer>(this);
			DataVisualizer->Init(VisQueriesArray);
		}
		else
		{
			// If the episode Id is not manually added, generate new unique id
			if (!bUseCustomEpisodeId)
			{
				EpisodeId = FIds::NewGuidInBase64Url();
			}

			if (bLogWorldState)
			{
				WorldStateLogger = NewObject<USLWorldLogger>(this);
				WorldStateLogger->Init(WriterType, FSLWorldWriterParams(
					LinearDistance, AngularDistance, TaskId, EpisodeId, ServerIp, ServerPort, bOverwriteWorldState));
			}

			if (bLogEventData)
			{
				EventDataLogger = NewObject<USLEventLogger>(this);
				EventDataLogger->Init(ExperimentTemplateType, FSLEventWriterParams(TaskId, EpisodeId),
					bLogContactEvents, bLogSupportedByEvents, bLogGraspEvents, bLogPickAndPlaceEvents, bLogSlicingEvents, bWriteTimelines);
			}
		}

		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start loggers
void ASLManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Reset world time
		if(bResetStartTime)
		{
			GetWorld()->TimeSeconds = 0.f;
		}

		if(bLogMetadata && MetadataLogger)
		{
			MetadataLogger->Start(TaskDescription);
		}
		else if(bLogEditorData && EditorLogger)
		{
			EditorLogger->Start(FSLEditorLoggerParams(bOverwriteEditorData,
				bWriteSemanticMap,
				bClearAllTags,
				TagTypeToClear,
				TagKeyToClear,
				bWriteClassTags,
				bWriteUniqueIdTags,
				bWriteUniqueMaskColors,
				MinColorManhattanDistance,
				bUseRandomColorGeneration));
			Finish(GetWorld()->GetTimeSeconds(),false); // Finish the manager directly
			//EditorLogger->Finish(); // Quit the editor before the manager finishes
		}
		else if (bLogVisionData && VisionDataLogger)
		{
			// Start the vision data logger
			VisionDataLogger->Start(EpisodeId);
		}
		else if (bVisualizeData && DataVisualizer)
		{
			DataVisualizer->Start(UserInputActionName);
		}
		else
		{
			// Start world state logger
			if (bLogWorldState && WorldStateLogger)
			{
				WorldStateLogger->Start(WorldStateUpdateRate);
			}

			// Start event data logger
			if (bLogEventData && EventDataLogger)
			{
				EventDataLogger->Start();
			}
		}
		// Mark manager as started
		bIsStarted = true;
	}
}

// Finish loggers
void ASLManager::Finish(const float Time, bool bForced)
{
	if (!bIsFinished && (bIsStarted || bIsInit))
	{
		if(bLogMetadata && MetadataLogger)
		{
			MetadataLogger->Finish(bForced);
		}
		else if(bLogEditorData && EditorLogger)
		{
			EditorLogger->Finish(bForced);
		}
		else if(bLogVisionData && VisionDataLogger)
		{
			VisionDataLogger->Finish(bForced);
		}
		else if (bVisualizeData && DataVisualizer)
		{
			DataVisualizer->Finish(bForced);
		}
		else
		{
			if (bLogWorldState && WorldStateLogger)
			{
				WorldStateLogger->Finish(bForced);
			}

			if (bLogEventData && EventDataLogger)
			{
				EventDataLogger->Finish(Time, bForced);
			}
		}
		
		// Delete the semantic items content instance
		FSLEntitiesManager::DeleteInstance();

		// Mark manager as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Logger Properties */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bUseCustomEpisodeId))
	{
		if (bUseCustomEpisodeId) { EpisodeId = FIds::NewGuidInBase64Url(); }
		else { EpisodeId = TEXT(""); };
	}
	
	/* Start Properties*/
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtBeginPlay))
	{
		if (bStartAtBeginPlay) {bStartAtFirstTick = false; bStartWithDelay = false; bStartFromUserInput = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtFirstTick))
	{
		if (bStartAtFirstTick) {bStartAtBeginPlay = false; bStartWithDelay = false; bStartFromUserInput = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartWithDelay))
	{
		if (bStartWithDelay) {bStartAtBeginPlay = false; bStartAtFirstTick = false; bStartFromUserInput = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartFromUserInput))
	{
		if (bStartFromUserInput) {bStartAtBeginPlay = false;  bStartWithDelay = false; bStartAtFirstTick = false;}
	}

	/* Metadata Properties */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogMetadata))
	{
		if (bLogMetadata) 
		{
			bLogWorldState = false;
			bLogEventData = false;
			bLogEditorData = false;
			bLogVisionData = false;
			bVisualizeData = false;
		};
	}

	/* Editor Properties */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogEditorData))
	{
		if (bLogEditorData) 
		{
			bLogWorldState = false;
			bLogEventData = false;
			bLogMetadata = false;
			bLogVisionData = false;
			bVisualizeData = false;
		};
	}

	/* World State / Event Logger Properties */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogWorldState))
	{
		if (bLogWorldState) {bLogEditorData = false; bLogMetadata = false; bLogVisionData = false; bVisualizeData = false;};
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogEventData))
	{
		if (bLogEventData) {bLogEditorData = false; bLogMetadata = false; bLogVisionData = false; bVisualizeData = false;};
	}

	/* Vision Data Logger Properties */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogVisionData))
	{
		if (bLogVisionData) 
		{
			bUseCustomEpisodeId = true;
			bLogEditorData = false;
			bLogMetadata = false;
			bLogWorldState = false;
			bLogEventData = false;
			bVisualizeData = false;
		};
	}
	
	/* Editor Logger Properties*/
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogEditorData))
	{
		if (bLogEditorData) 
		{ 
			bLogVisionData = false;
			bLogMetadata = false;
			bLogWorldState = false;
			bLogEventData = false;
			bVisualizeData = false;
		};
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bClearAllTags))
	{
		if (bClearAllTags) {bWriteClassTags = false; bWriteUniqueIdTags = false; bWriteUniqueMaskColors = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bWriteClassTags))
	{
		if (bWriteClassTags) {bClearAllTags = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bWriteUniqueIdTags))
	{
		if (bWriteUniqueIdTags) {bClearAllTags = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bWriteUniqueMaskColors))
	{
		if (bWriteUniqueMaskColors) {bClearAllTags = false;}
	}

	/* Data Visualizer Properties */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bVisualizeData))
	{
		if (bVisualizeData) 
		{ 
			bUseCustomEpisodeId = true;
			bLogVisionData = false;
			bLogEditorData = false;
			bLogMetadata = false;
			bLogWorldState = false;
			bLogEventData = false; 
		};
	}
}

// Called by the editor to query whether a property of this object is allowed to be modified.
bool ASLManager::CanEditChange(const UProperty* InProperty) const
{
	// Get parent edit property
	const bool ParentVal = Super::CanEditChange(InProperty);

	// Get the property name
	const FName PropertyName = InProperty->GetFName();

	// HostIP and HostPort can only be edited if the world state writer is of type Mongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, ServerIp))
	{
		return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, ServerPort))
	{
		return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogMetadata))
	{
		return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	}

	return ParentVal;
}
#endif // WITH_EDITOR

// Bind user inputs
void ASLManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(UserInputActionName, IE_Pressed, this, &ASLManager::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLManager::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLManager::Start();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] SemLog started.."), GetWorld()->GetTimeSeconds()));
	}
	else if(bIsStarted && !bIsFinished)
	{		
		ASLManager::Finish(GetWorld()->GetTimeSeconds());
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] SemLog finished.."), GetWorld()->GetTimeSeconds()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] Something went wrong, try again.."), GetWorld()->GetTimeSeconds()));
	}
}
