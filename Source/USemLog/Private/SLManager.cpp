// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManager.h"
#include "SLMap.h"
#include "Ids.h"
#if WITH_SL_VIS
#include "SLVisManager.h"
#endif //WITH_SL_VIS

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
	bUseCustomEpisodeId = false;
	EpisodeId = TEXT("autogen");
	LogDirectory = TEXT("SemLog");
	bStartAtBeginPlay = true;
	bStartAtFirstTick = false;
	bStartWithDelay = false;
	StartDelay = 0.5f;

	// Raw data logger default values
	bLogRawData = true;
	UpdateRate = 0.0f;
	DistanceThreshold = 0.5f;
	bLogToJson = true;
	bLogToBson = false;
	bLogToMongo = false;
	MongoIP = TEXT("127.0.0.1.");
	MongoPort = 27017;

	// Events logger default values
	bLogEventData = true;
	bWriteTimelines = true;
	ExperimentTemplateType = EOwlExperimentTemplate::Default;

	// Vision data logger default values
	bLogVisionData = true;

#if WITH_EDITOR
	// Make sprite smaller
	SpriteScale = 0.5;
#endif // WITH_EDITOR
}

// Sets default values
ASLManager::~ASLManager()
{
	if (!bIsFinished)
	{
		ASLManager::Finish();
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// TODO test init here
	// See if all other components are init when this is triggered (should be)
	//ASLManager::Init();
	
	// use PrimaryComponentTick.bStartWithTickEnabled = false; 
	// to avoid disabling tick on BeginPlay where the order of calls is not known
	// see USLVisManager as example
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		ASLManager::Init();
		ASLManager::Start();
	}
	else if (bStartAtFirstTick)
	{
		FTimerDelegate TimerDelegateNextTick;
		TimerDelegateNextTick.BindLambda([this]
		{
			ASLManager::Init();
			ASLManager::Start();
		});
		GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
	}
	else if (bStartWithDelay)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegateDelay;
		TimerDelegateDelay.BindLambda([this]
		{
			ASLManager::Init();
			ASLManager::Start();
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartDelay, false);
	}
}

// Called when actor removed from game or game ended
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		ASLManager::Finish();
	}
}

// Init loggers
void ASLManager::Init()
{
	if (!bIsInit)
	{
		// Init the semantic items content singleton
		if (!FSLMap::GetInstance()->IsInit())
		{
			FSLMap::GetInstance()->LoadData(GetWorld());
		}

		// If the episode Id is not manually added, generate new unique id
		if (!bUseCustomEpisodeId)
		{
			// Generate unique id for the episode
			EpisodeId = FIds::NewGuidInBase64Url();
		}
				
		if (bLogRawData)
		{
			// Create and init raw data logger
			RawDataLogger = NewObject<USLRawDataLogger>(this);
			RawDataLogger->Init(DistanceThreshold);

			// Set log type
			if (bLogToJson)
			{
				RawDataLogger->SetLogToJson(LogDirectory, EpisodeId);
			}
			if (bLogToBson)
			{
				RawDataLogger->SetLogToBson(LogDirectory, EpisodeId);
			}
			if (bLogToMongo)
			{
				RawDataLogger->SetLogToMongo(LogDirectory, EpisodeId, MongoIP, MongoPort);
			}
		}

		if (bLogEventData)
		{
			// Create and init event data logger
			EventDataLogger = NewObject<USLEventDataLogger>(this);
			EventDataLogger->Init(LogDirectory, EpisodeId, ExperimentTemplateType, bWriteTimelines);
			// TODO init all listeners here and not on their own (begin play etc.)
		}

#if WITH_SL_VIS
		if (bLogVisionData)
		{
			// Cache and init all vision logger components

			//for (TObjectIterator<USLVisManager> ObjItr; ObjItr; ++ObjItr)
			//{
			//	ObjItr->Init(LogDirectory, EpisodeId);
			//	VisionDataLoggerManagers.Add(*ObjItr);
			//}
			for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				// Iterate components of the actor, check for vision capture manager
				for (auto& CompItr : ActorItr->GetComponents())
				{
					if (USLVisManager* VisMan = Cast<USLVisManager>(CompItr))
					{
						VisMan->Init(LogDirectory, EpisodeId);
						VisionDataLoggerManagers.Add(VisMan);
					}
				}
			}
		}
#endif //WITH_SL_VIS

		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start loggers
void ASLManager::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start raw data logger
		if (bLogRawData && RawDataLogger)
		{
			RawDataLogger->Start(UpdateRate);
		}

		// Start event data logger
		if (bLogEventData && EventDataLogger)
		{
			EventDataLogger->Start();
		}

#if WITH_SL_VIS
		// Start vision loggers
		for (auto& VisMan : VisionDataLoggerManagers)
		{
			VisMan->Start();
		}
#endif //WITH_SL_VIS

		// Mark manager as started
		bIsStarted = true;
	}
}

// Finish loggers
void ASLManager::Finish()
{
	if (bIsInit || bIsStarted)
	{
		if (RawDataLogger)
		{
			RawDataLogger->Finish();
		}

		if (EventDataLogger)
		{
			EventDataLogger->Finish();
		}

#if WITH_SL_VIS
		for (auto& VisMan : VisionDataLoggerManagers)
		{
			VisMan->Finish();
		}
#endif //WITH_SL_VIS
		
		// Delete the semantic items content instance
		FSLMap::DeleteInstance();

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

	// Radio button style between bLogToJson, bLogToBson, bLogToMongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToJson))
	{
		if (bLogToJson){bLogToBson = false; bLogToMongo = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToBson))
	{
		if (bLogToBson){bLogToJson = false; bLogToMongo = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogToMongo))
	{
		if (bLogToMongo){bLogToJson = false; bLogToBson = false;}
	}

	// Radio button style between bStartAtBeginPlay, bStartAtFirstTick, bStartWithDelay
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtBeginPlay))
	{
		if (bStartAtBeginPlay){bStartAtFirstTick = false; bStartWithDelay = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtFirstTick))
	{
		if (bStartAtFirstTick){bStartAtBeginPlay = false; bStartWithDelay = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartWithDelay))
	{
		if (bStartWithDelay){bStartAtBeginPlay = false; bStartAtFirstTick = false;}
	}

	// Writing timelines
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogEventData))
	{
		if (!bLogEventData){bWriteTimelines = false;}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogEventData))
	{
		if (bLogEventData)
		{
			bWriteTimelines = true;
		}
	}
}
#endif // WITH_EDITOR