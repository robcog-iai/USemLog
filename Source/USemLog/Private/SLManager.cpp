// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLManager.h"
#include "SLEntitiesManager.h"
#include "Ids.h"
#if SL_WITH_SLVIS
#include "Engine/DemoNetDriver.h"
#endif //SL_WITH_SLVIS

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
	bUseCustomLocation = false;
	Location = TEXT("SemLog");
	bUseCustomEpisodeId = false;
	EpisodeId = TEXT("autogen");
	bStartAtBeginPlay = true;
	bStartAtFirstTick = false;
	bStartWithDelay = false;
	StartDelay = 0.5f;
	bStartFromUserInput = false;
	StartInputActionName = "SLStart";
	FinishInputActionName = "SLFinish";

	// World state logger default values
	bLogWorldState = true;
	UpdateRate = 0.0f;
	LinearDistance = 0.5f; // cm
	AngularDistance = 0.1f; // rad
	WriterType = ESLWorldStateWriterType::MongoC;
	ServerIp = TEXT("127.0.0.1");
	ServerPort = 27017;
	bIncludeAllData = false;

	// Events logger default values
	bLogEventData = true;
	bLogContactEvents = true;
	bLogSupportedByEvents = true;
	bLogGraspEvents = true;
	bWriteTimelines = true;
	bWriteMetadata = true;
	ExperimentTemplateType = ESLOwlExperimentTemplate::Default;

	// Vision data logger default values
	bLogVisionData = true;
	MaxRecordHz = 90.f;
	MinRecordHz = 30.f;

#if WITH_EDITOR
	// Make manager sprite smaller
	SpriteScale = 0.5;
#endif // WITH_EDITOR
}

// Sets default values
ASLManager::~ASLManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		ASLManager::Finish(-1.f, true);
	}
}

// Allow actors to initialize themselves on the C++ side
void ASLManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Init loggers
	ASLManager::Init();
}

// Called when the game starts or when spawned
void ASLManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		ASLManager::Start();
	}
	else if (bStartAtFirstTick)
	{
		FTimerDelegate TimerDelegateNextTick;
		TimerDelegateNextTick.BindLambda([this]
		{
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
			ASLManager::Start();
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartDelay, false);
	}
	else if (bStartFromUserInput)
	{
		// Bind user input
		ASLManager::SetupInputBindings();
	}
}

// Called when actor removed from game or game ended
void ASLManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		ASLManager::Finish(GetWorld()->GetTimeSeconds());
	}
}

// Init loggers
void ASLManager::Init()
{
#if SL_WITH_SLVIS
	// TODO needed, since un-replicating the manager did not seem to work
	//SetReplicateMovement(false);
	//SetReplicates(false);
	// Init can be called even if it is a demo replay, skip if it is the case
	if (GetWorld()->DemoNetDriver && GetWorld()->DemoNetDriver->IsPlaying())
	{
		return;
	}
#endif // SL_WITH_SLVIS

	if (!bIsInit)
	{
		// Init the semantic items content singleton
		FSLEntitiesManager::GetInstance()->Init(GetWorld());

		// If the episode Id is not manually added, generate new unique id
		if (!bUseCustomEpisodeId)
		{
			EpisodeId = FIds::NewGuidInBase64Url();
		}

		if (bLogWorldState)
		{
			// Create and init world state logger
			WorldStateLogger = NewObject<USLWorldStateLogger>(this);
			WorldStateLogger->Init(WriterType, FSLWorldStateWriterParams(
				LinearDistance, AngularDistance, Location, EpisodeId, ServerIp, ServerPort));
		}

		if (bLogEventData)
		{
			// Create and init event data logger
			EventDataLogger = NewObject<USLEventLogger>(this);
			EventDataLogger->Init(ExperimentTemplateType, FSLEventWriterParams(Location, EpisodeId, ServerIp, ServerPort),
				bLogContactEvents, bLogSupportedByEvents, bLogGraspEvents, bWriteTimelines, bWriteMetadata);
		}

		if (bLogVisionData)
		{
			// Create and init vision logger
			VisionDataLogger = NewObject<USLVisionLogger>(this);
			VisionDataLogger->Init(MaxRecordHz, MinRecordHz);
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
		GetWorld()->TimeSeconds = 0.f;

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

		// Start the vision data logger
		if (bLogVisionData && VisionDataLogger)
		{
			VisionDataLogger->Start(EpisodeId);
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
		if (WorldStateLogger)
		{
			WorldStateLogger->Finish(bForced);
		}

		if (EventDataLogger)
		{
			EventDataLogger->Finish(Time, bForced);
		}
		
		if (VisionDataLogger)
		{
			VisionDataLogger->Finish(bForced);
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

	// Radio button style between start flags
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bStartAtBeginPlay))
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
	
	// Generate an editable unique id
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bUseCustomEpisodeId))
	{
		if (bUseCustomEpisodeId) { EpisodeId = FIds::NewGuidInBase64Url(); }
		else { EpisodeId = TEXT("autogen"); };
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
		return (WriterType == ESLWorldStateWriterType::MongoCxx) || (WriterType == ESLWorldStateWriterType::MongoC);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, ServerPort))
	{
		return (WriterType == ESLWorldStateWriterType::MongoCxx) || (WriterType == ESLWorldStateWriterType::MongoC);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bIncludeAllData))
	{
		return (WriterType == ESLWorldStateWriterType::MongoCxx) || (WriterType == ESLWorldStateWriterType::MongoC);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogVisionData))
	{
#if SL_WITH_SLVIS
		return true;
#else
		return false;
#endif //SL_WITH_SLVIS
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
			IC->BindAction(StartInputActionName, IE_Pressed, this, &ASLManager::StartFromInput);
			IC->BindAction(FinishInputActionName, IE_Pressed, this, &ASLManager::FinishFromInput);
		}
	}
}

// Start input binding
void ASLManager::StartFromInput()
{
	ASLManager::Start();
}

// Start input binding
void ASLManager::FinishFromInput()
{	
	ASLManager::Finish(GetWorld()->GetTimeSeconds());
}