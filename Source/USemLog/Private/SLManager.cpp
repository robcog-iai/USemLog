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
	bStartFromUserInput = false;
	StartInputActionName = "SLStart";
	FinishInputActionName = "SLFinish";

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
	if (!bIsFinished && !IsTemplate())
	{
		UE_LOG(LogSL, Error, TEXT(">>%s::%d Called in destructor finishing events with time -1.f"),
			TEXT(__FUNCTION__), __LINE__);
		ASLManager::Finish(-1.f);
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
	UE_LOG(LogSL, Warning, TEXT("%s %d"), TEXT(__FUNCTION__), __LINE__);
	if (!bIsInit)
	{
		UE_LOG(LogSL, Warning, TEXT("\t\t%s %d"), TEXT(__FUNCTION__), __LINE__);
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
			WorldStateLogger->Init(bLogVisionData, WriterType, DistanceStepSize, RotationStepSize,
				EpisodeId, Location, HostIP, HostPort);
		}

		if (bLogEventData)
		{
			// Create and init event data logger
			EventDataLogger = NewObject<USLEventLogger>(this);
			EventDataLogger->Init(ExperimentTemplateType, EpisodeId, Location,
				bLogContactEvents, bLogSupportedByEvents, bLogGraspEvents, bWriteTimelines);
		}

		// Mark manager as initialized
		bIsInit = true;
	}
}

// Start loggers
void ASLManager::Start()
{
	UE_LOG(LogSL, Warning, TEXT("%s %d"), TEXT(__FUNCTION__), __LINE__);
	if (!bIsStarted && bIsInit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("START"));
		UE_LOG(LogSL, Warning, TEXT("\t\t%s %d"), TEXT(__FUNCTION__), __LINE__);
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

		// Mark manager as started
		bIsStarted = true;
	}
}

// Finish loggers
void ASLManager::Finish(const float Time)
{
	UE_LOG(LogSL, Warning, TEXT("%s %d"), TEXT(__FUNCTION__), __LINE__);
	if (bIsInit || bIsStarted)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FINISH"));
		UE_LOG(LogSL, Warning, TEXT("\t\t%s %d"), TEXT(__FUNCTION__), __LINE__);
		if (WorldStateLogger)
		{
			WorldStateLogger->Finish();
		}

		if (EventDataLogger)
		{
			EventDataLogger->Finish(Time);
		}
		
		// Delete the semantic items content instance
		// TODO keep this because of the vision logger
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

// Bind user inputs
void ASLManager::SetupInputBindings()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
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