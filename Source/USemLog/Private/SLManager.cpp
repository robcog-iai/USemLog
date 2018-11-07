// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManager.h"
#include "SLMappings.h"
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
	Location = TEXT("SemLog");
	bStartAtBeginPlay = true;
	bStartAtFirstTick = false;
	bStartWithDelay = false;
	StartDelay = 0.5f;

	// World state logger default values
	bLogWorldState = true;
	UpdateRate = 0.0f;
	DistanceStepSize = 0.5f;
	RotationStepSize = 0.5f;
	WriterType = ESLWorldStateWriterType::Json;
	HostIP = TEXT("127.0.0.1.");
	HostPort = 27017;

	// Events logger default values
	bLogEventData = true;
	bLogContactEvents = true;
	bLogSupportedByEvents = true;
	bLogGraspEvents = true;
	bWriteTimelines = true;
	ExperimentTemplateType = ESLOwlExperimentTemplate::Default;

	// Vision data logger default values
	bLogVisionData = true;

#if WITH_EDITOR
	// Make manager sprite smaller
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
		FSLMappings::GetInstance()->Init(GetWorld());

		// If the episode Id is not manually added, generate new unique id
		if (!bUseCustomEpisodeId)
		{
			// Generate unique id for the episode
			EpisodeId = FIds::NewGuidInBase64Url();
		}
				
		if (bLogWorldState)
		{
			// Create and init world state logger
			WorldStateLogger = NewObject<USLWorldStateLogger>(this);
			WorldStateLogger->Init(WriterType, DistanceStepSize, RotationStepSize,
				EpisodeId, Location, HostIP, HostPort);
		}

		if (bLogEventData)
		{
			// Create and init event data logger
			EventDataLogger = NewObject<USLEventLogger>(this);
			EventDataLogger->Init(ExperimentTemplateType, EpisodeId, Location,
				bLogContactEvents, bLogSupportedByEvents, bLogGraspEvents, bWriteTimelines);
		}

		if (bLogVisionData)
		{
#if WITH_SL_VIS
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
						VisMan->Init(Location, EpisodeId);
						VisionDataLoggers.Add(VisMan);
					}
				}
			}
#endif //WITH_SL_VIS
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
		// Start world state logger
		if (bLogWorldState && WorldStateLogger)
		{
			WorldStateLogger->Start(UpdateRate);
		}

		// Start event data logger
		if (bLogEventData && EventDataLogger)
		{
			EventDataLogger->Start();
		}

#if WITH_SL_VIS
		// Start vision loggers
		for (auto& VisMan : VisionDataLoggers)
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
		if (WorldStateLogger)
		{
			WorldStateLogger->Finish();
		}

		if (EventDataLogger)
		{
			EventDataLogger->Finish();
		}

#if WITH_SL_VIS
		for (auto& VisMan : VisionDataLoggers)
		{
			VisMan->Finish();
		}
#endif //WITH_SL_VIS
		
		// Delete the semantic items content instance
		FSLMappings::DeleteInstance();

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

	// Radio button style between bStartAtBeginPlay, bStartAtFirstTick, bStartWithDelay
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtBeginPlay))
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

// Called by the editor to query whether a property of this object is allowed to be modified.
bool ASLManager::CanEditChange(const UProperty* InProperty) const
{
	// Get parent edit property
	const bool ParentVal = Super::CanEditChange(InProperty);

	// Get the property name
	const FName PropertyName = InProperty->GetFName();

	// HostIP and HostPort can only be edited if the world state writer is of type Mongo
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, HostIP))
	{
		return WriterType == ESLWorldStateWriterType::Mongo;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, HostPort))
	{
		return WriterType == ESLWorldStateWriterType::Mongo;
	}

	return ParentVal;
}
#endif // WITH_EDITOR