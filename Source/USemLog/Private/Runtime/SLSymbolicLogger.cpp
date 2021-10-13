// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Runtime/SLSymbolicLogger.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualComponent.h"

#include "Utils/SLUuid.h"
#include "EngineUtils.h"
#include "TimerManager.h"
//#include "Misc/Paths.h"
//#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

#include "Events/SLGoogleCharts.h"

#include "Events/SLContactEventHandler.h"
#include "Events/SLManipulatorContactEventHandler.h"
#include "Events/SLGraspEventHandler.h"
#include "Events/SLReachAndPreGraspEventHandler.h"
#include "Events/SLPickAndPlaceEventsHandler.h"
#include "Events/SLContainerEventHandler.h"

#include "Monitors/SLContactMonitorInterface.h"
#include "Monitors/SLManipulatorMonitor.h"
#include "Monitors/SLReachAndPreGraspMonitor.h"
#include "Monitors/SLPickAndPlaceMonitor.h"
#include "Monitors/SLContainerMonitor.h"

#include "Owl/SLOwlExperimentStatics.h"

#if SL_WITH_MC_GRASP
#include "Events/SLFixationGraspEventHandler.h"
#include "MCGraspFixation.h"
#endif // SL_WITH_MC_GRASP

#if SL_WITH_SLICING
#include "Events/SLSlicingEventHandler.h"
#include "SlicingBladeComponent.h"
#endif // SL_WITH_SLICING

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLSymbolicLogger::ASLSymbolicLogger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	bUseIndependently = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLSymbolicLogger"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Force call on finish
ASLSymbolicLogger::~ASLSymbolicLogger()
{
}

// Allow actors to initialize themselves on the C++ side
void ASLSymbolicLogger::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (bUseIndependently)
	{
		InitImpl();
	}
}

// Called when the game starts or when spawned
void ASLSymbolicLogger::BeginPlay()
{
	Super::BeginPlay();
	if (bUseIndependently)
	{
		if (StartParameters.StartTime == ESLLoggerStartTime::AtBeginPlay)
		{
			StartImpl();
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::AtNextTick)
		{
			FTimerDelegate TimerDelegateNextTick;
			TimerDelegateNextTick.BindLambda([this] {StartImpl(); });
			GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::AfterDelay)
		{
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegateDelay;
			TimerDelegateDelay.BindLambda([this] {StartImpl(); });
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, StartParameters.StartDelay, false);
		}
		else if (StartParameters.StartTime == ESLLoggerStartTime::FromUserInput)
		{
			SetupInputBindings();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Logger (%s) StartImpl() will not be called.."),
				*FString(__func__), __LINE__, *GetName());
		}
	}
}

// Called when actor removed from game or game ended
void ASLSymbolicLogger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bUseIndependently && !bIsFinished)
	{
		FinishImpl();
	}
}

// Init logger (called when the logger is synced externally)
void ASLSymbolicLogger::Init(const FSLSymbolicLoggerParams& InLoggerParameters,
	const FSLLoggerLocationParams& InLocationParameters)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	LoggerParameters = InLoggerParameters;
	LocationParameters = InLocationParameters;
	InitImpl();
}

// Start logger (called when the logger is synced externally)
void ASLSymbolicLogger::Start()
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	StartImpl();
}

// Finish logger (called when the logger is synced externally) (bForced is true if called from destructor)
void ASLSymbolicLogger::Finish(bool bForced)
{
	if (bUseIndependently)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is set to work independetly, external calls will have no effect.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	FinishImpl();
}

// Init logger (called when the logger is used independently)
void ASLSymbolicLogger::InitImpl()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!LocationParameters.bUseCustomTaskId)
	{
		LocationParameters.TaskId = FSLUuid::NewGuidInBase64Url();
	}

	if (!LocationParameters.bUseCustomEpisodeId)
	{
		LocationParameters.EpisodeId = FSLUuid::NewGuidInBase64Url();
	}

	// Make sure the individual manager is set and loaded
	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!IndividualManager->IsLoaded() && !IndividualManager->Load(true))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager (%s) could not load the individual manager (%s).."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
		return;
	}

	// Create the document template
	ExperimentDoc = CreateEventsDocTemplate(ESLOwlExperimentTemplate::Default, LocationParameters.EpisodeId);

	// Setup monitors
	if (LoggerParameters.EventsSelection.bSelectAll)
	{
		InitContactMonitors();
		InitReachAndPreGraspMonitors();
		InitManipulatorContactAndGraspMonitors();
		InitPickAndPlaceMonitors();
		//InitManipulatorGraspFixationMonitors();
		/*InitManipulatorContainerMonitors();
		InitSlicingMonitors();*/
	}
	else
	{
		/* Basic contact */
		if (LoggerParameters.EventsSelection.bContact)
		{
			InitContactMonitors();
		}

		/* Reach */
		if (LoggerParameters.EventsSelection.bReachAndPreGrasp)
		{
			if (LoggerParameters.EventsSelection.bGrasp)
			{
				InitReachAndPreGraspMonitors();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Reach monitors only work if grasp events are enabled.."),
					*FString(__FUNCTION__), __LINE__);
			}
		}

		/* Hand contact and/or grasp*/
		if (LoggerParameters.EventsSelection.bManipulatorContact || LoggerParameters.EventsSelection.bGrasp)
		{
			InitManipulatorContactAndGraspMonitors();
		}

		/* Pick and place */
		if (LoggerParameters.EventsSelection.bPickAndPlace)
		{
			if (LoggerParameters.EventsSelection.bGrasp)
			{
				InitPickAndPlaceMonitors();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Pick-and-Place monitors only work if grasp events are enabled.."),
					*FString(__FUNCTION__), __LINE__);
			}
		}

		//if (LoggerParameters.EventsSelection.bSlicing)
		//{
		//	InitSlicingMonitors();
		//}
	}

	if (LoggerParameters.bPublishToROS)
	{
		InitROSPublisher();
	}

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully initialized at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Start logger (called when the logger is used independently)
void ASLSymbolicLogger::StartImpl()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Start handlers
	for (auto& EvHandler : EventHandlers)
	{
		// Subscribe for given semantic events
		EvHandler->Start();

		// Bind resulting events
		EvHandler->OnSemanticEvent.BindUObject(this, &ASLSymbolicLogger::SemanticEventFinishedCallback);
	}

	// Start the pick and place Monitors (subscribes for grasp events)
	for (auto& Monitor : PickAndPlaceMonitors)
	{
		Monitor->Start();
	}

	// Start the reach Monitors
	for (auto& Monitor : ReachAndPreGraspMonitors)
	{
		Monitor->Start();
	}

	// Start the manipulator contact and grasp monitors (start after subscribers)
	for (auto& Monitor : ManipulatorContactAndGraspMonitors)
	{
		Monitor->Start();
	}

	// Start the semantic overlap areas
	for (auto& Monitor : ContactMonitors)
	{
		Monitor->Start();
	}

	//// Start the container Monitors
	//for (auto& Monitor : ContainerMonitors)
	//{
	//	Monitor->Start();
	//}

	EpisodeStartTime = GetWorld()->GetTimeSeconds();

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully started at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
void ASLSymbolicLogger::FinishImpl(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit && !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) is not initialized nor started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (GetWorld())
	{
		EpisodeEndTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the world pointer.."),	*FString(__FUNCTION__), __LINE__);
		return;
	}

	// Finish handlers pending events
	for (auto& EvHandler : EventHandlers)
	{
		EvHandler->Finish(EpisodeEndTime, bForced);
	}
	EventHandlers.Empty();

	// Finish semantic overlap events publishing
	for (auto& SLContactMonitor : ContactMonitors)
	{
		SLContactMonitor->Finish();
	}
	ContactMonitors.Empty();

	// Finish the reach Monitors
	for (auto& SLReachAndPreGraspMonitor : ReachAndPreGraspMonitors)
	{
		SLReachAndPreGraspMonitor->Finish();
	}
	ReachAndPreGraspMonitors.Empty();

	// Finish the grasp Monitors
	for (auto& SLManipulatorMonitor : ManipulatorContactAndGraspMonitors)
	{
		SLManipulatorMonitor->Finish(bForced);
	}
	ManipulatorContactAndGraspMonitors.Empty();

	// Finish the pick and place Monitors
	for (auto& SLPapMonitor : PickAndPlaceMonitors)
	{
		SLPapMonitor->Finish(EpisodeEndTime);
	}
	PickAndPlaceMonitors.Empty();

	//// Finish the container Monitors
	//for (auto& SLContainerMonitor : ContainerMonitors)
	//{
	//	SLContainerMonitor->Finish();
	//}
	//ContainerMonitors.Empty();

	// Create the experiment owl doc	
	if (ExperimentDoc.IsValid())
	{
		TArray<FString> SubActionIds;		
		for (const auto& Ev : FinishedEvents)
		{
			Ev->AddToOwlDoc(ExperimentDoc.Get());
			SubActionIds.Add(Ev->Id);
		}

		// Add stored unique timepoints to doc
		ExperimentDoc->AddTimepointIndividuals();

		// Add stored unique objects to doc
		//ExperimentDoc->AddObjectIndividuals();

		// Add experiment individual to doc	(metadata)	
		ExperimentDoc->AddExperimentIndividual(SubActionIds, LocationParameters.SemanticMapId, LocationParameters.TaskId);
	}

	// Write events to file
	WriteToFile();

#if SL_WITH_ROSBRIDGE
	// Finish ROS Connection
	ROSPrologClient->Disconnect();
#endif // SL_WITH_ROSBRIDGE

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Symbolic logger (%s) succesfully finished at %.2f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Bind user inputs
void ASLSymbolicLogger::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParameters.UserInputActionName, IE_Pressed, this, &ASLSymbolicLogger::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLSymbolicLogger::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLSymbolicLogger::StartImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLSymbolicLogger::FinishImpl();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] Symbolic logger (%s) logger finished, or not initalized.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

// Called when a semantic event is done
void ASLSymbolicLogger::SemanticEventFinishedCallback(TSharedPtr<ISLEvent> Event)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s::%d %s"), *FString(__func__), __LINE__, *Event->ToString()));
	//UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), *FString(__func__), __LINE__, *Event->ToString());
	FinishedEvents.Add(Event);

#if SL_WITH_ROSBRIDGE
	if (LoggerParameters.bPublishToROS)
	{
		ROSPrologClient->AddEventQuery(Event);
	}
#endif // SL_WITH_ROSBRIDGE
}

// Write data to file
void ASLSymbolicLogger::WriteToFile()
{
	const FString DirPath = FPaths::ProjectDir() + "/SL/Tasks/" + LocationParameters.TaskId /*+ TEXT("/Episodes/")*/ + "/";

	// Write events timelines to file
	if (LoggerParameters.bWriteTimelines)
	{
		FSLGoogleChartsParameters Params;
		Params.bTooltips = true;
		Params.StartTime = EpisodeStartTime;
		Params.EndTime = EpisodeEndTime;
		Params.TaskId = LocationParameters.TaskId;
		Params.EpisodeId = LocationParameters.EpisodeId;
		Params.bOverwrite = LocationParameters.bOverwrite;
		Params.EventsSelection = LoggerParameters.TimelineEventsSelection;
		FSLGoogleCharts::WriteTimelines(FinishedEvents, DirPath, LocationParameters.EpisodeId, Params);
	}


	// Write experiment owl to file
	FSLOwlExperimentStatics::WriteToFile(ExperimentDoc, DirPath, LocationParameters.bOverwrite);

	//// Write owl data to file
	//if (ExperimentDoc.IsValid())
	//{
	//	// Write experiment to file
	//	FString FullFilePath = DirPath + LocationParameters.EpisodeId + TEXT("_ED.owl");
	//	FPaths::RemoveDuplicateSlashes(FullFilePath);
	//	if (!FPaths::FileExists(FullFilePath) || LocationParameters.bOverwrite)
	//	{
	//		FFileHelper::SaveStringToFile(ExperimentDoc->ToString(), *FullFilePath);					
	//	}
	//}
}

// Create events doc template
TSharedPtr<FSLOwlExperiment> ASLSymbolicLogger::CreateEventsDocTemplate(ESLOwlExperimentTemplate TemplateType, const FString& InDocId)
{
	return FSLOwlExperimentStatics::CreateDefaultExperiment(InDocId, "log", "ameva_log");
	//// Create unique semlog id for the document
	//const FString DocId = InDocId.IsEmpty() ? FSLUuid::NewGuidInBase64Url() : InDocId;

	//// Fill document with template values
	//if (TemplateType == ESLOwlExperimentTemplate::Default)
	//{
	//	return FSLOwlExperimentStatics::CreateDefaultExperiment(DocId);
	//}
	//else if (TemplateType == ESLOwlExperimentTemplate::IAI)
	//{
	//	return FSLOwlExperimentStatics::CreateUEExperiment(DocId);
	//}
	//return MakeShareable(new FSLOwlExperiment());
}

// Get the reference or spawn a new individual manager
bool ASLSymbolicLogger::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLIndividualManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			IndividualManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	IndividualManager = GetWorld()->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return true;
}

// Helper function which checks if the individual data is loaded
bool ASLSymbolicLogger::IsValidAndLoaded(AActor* Actor)
{
	if (Actor == nullptr || !Actor->IsValidLowLevel() || Actor->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Actor not valid.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!GetWorld()->ContainsActor(Actor))
	{
		//UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not from this world.."), *FString(__func__), __LINE__, *Actor->GetName());
		return false;
	}
	if (UActorComponent* ActComp = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		if (CastChecked<USLIndividualComponent>(ActComp)->IsLoaded())
		{
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual is not loaded.."), *FString(__func__), __LINE__, *Actor->GetName());
		}
	}
	return false;
}

// Iterate contact monitors in the world
void ASLSymbolicLogger::InitContactMonitors()
{
	// Init all contact trigger handlers
	for (TObjectIterator<UShapeComponent> Itr; Itr; ++Itr)
	{
		//if (Itr->GetClass()->ImplementsInterface(USLContactMonitorInterface::StaticClass()))
		if (ISLContactMonitorInterface* ContactMonitor = Cast<ISLContactMonitorInterface>(*Itr))
		{
			if (IsValidAndLoaded(Itr->GetOwner()))
			{
				ContactMonitor->Init(LoggerParameters.EventsSelection.bSupportedBy);
				if (ContactMonitor->IsInit())
				{
					ContactMonitors.Emplace(ContactMonitor);

					// Create a contact event handler 
					TSharedPtr<FSLContactEventHandler> EvHandler = MakeShareable(new FSLContactEventHandler());
					EvHandler->Init(*Itr);
					EvHandler->EpisodeId = LocationParameters.EpisodeId;
					if (EvHandler->IsInit())
					{
						EventHandlers.Emplace(EvHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s::%s's handler could not be init.."),
							*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
					}
				}
			}
		}
	}
}

// Iterate and init the manipulator contact monitors in the world
void ASLSymbolicLogger::InitManipulatorContactAndGraspMonitors()
{
	// Init all grasp Monitors
	for (TObjectIterator<USLManipulatorMonitor> Itr; Itr; ++Itr)
	{
		if (IsValidAndLoaded(Itr->GetOwner()))
		{
			Itr->Init(LoggerParameters.EventsSelection.bGrasp, LoggerParameters.EventsSelection.bManipulatorContact);
			if (Itr->IsInit())
			{
				ManipulatorContactAndGraspMonitors.Emplace(*Itr);

				if (LoggerParameters.EventsSelection.bGrasp)
				{
					TSharedPtr<FSLGraspEventHandler> EvHandler = MakeShareable(new FSLGraspEventHandler());
					EvHandler->Init(*Itr);
					EvHandler->EpisodeId = LocationParameters.EpisodeId;
					if (EvHandler->IsInit())
					{
						EventHandlers.Add(EvHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s::%s's handler could not be init.."),
							*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
					}
				}

				if (LoggerParameters.EventsSelection.bManipulatorContact)
				{
					TSharedPtr<FSLManipulatorContactEventHandler> EvHandler = MakeShareable(new FSLManipulatorContactEventHandler());
					EvHandler->Init(*Itr);
					EvHandler->EpisodeId = LocationParameters.EpisodeId;
					if (EvHandler->IsInit())
					{
						EventHandlers.Add(EvHandler);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s::%s's handler could not be init.."),
							*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's monitor could not be init.."),
					*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
			}
		}
	}
}

// Iterate and init the manipulator fixation monitors in the world
void ASLSymbolicLogger::InitManipulatorGraspFixationMonitors()
{
#if SL_WITH_MC_GRASP
	// Init fixation grasp Monitors
	for (TObjectIterator<UMCGraspFixation> Itr; Itr; ++Itr)
	{
		if (IsValidAndLoaded(Itr->GetOwner()))
		{
			// Create a grasp event handler 
			TSharedPtr<FSLFixationGraspEventHandler> EvHandler = MakeShareable(new FSLFixationGraspEventHandler());
			EvHandler->Init(*Itr);
			EvHandler->EpisodeId = LocationParameters.EpisodeId;
			if (EvHandler->IsInit())
			{
				EventHandlers.Add(EvHandler);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s::%s's handler could not be init.."),
					*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
			}
		}
	}
#endif // SL_WITH_MC_GRASP
}

// Iterate and init the manipulator reach monitors
void ASLSymbolicLogger::InitReachAndPreGraspMonitors()
{
	for (TObjectIterator<USLReachAndPreGraspMonitor> Itr; Itr; ++Itr)
	{
		if (IsValidAndLoaded(Itr->GetOwner()))
		{
			Itr->Init();
			if (Itr->IsInit())
			{
				ReachAndPreGraspMonitors.Emplace(*Itr);
				TSharedPtr<FSLReachAndPreGraspEventHandler> EvHandler = MakeShareable(new FSLReachAndPreGraspEventHandler());
				EvHandler->Init(*Itr);
				EvHandler->EpisodeId = LocationParameters.EpisodeId;
				if (EvHandler->IsInit())
				{
					EventHandlers.Add(EvHandler);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s::%s's handler could not be init.."),
						*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's monitor could not be init.."),
					*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
			}
		}
	}
}

// Iterate and init the manipulator container monitors
void ASLSymbolicLogger::InitManipulatorContainerMonitors()
{
	//for (TObjectIterator<USLContainerMonitor> Itr; Itr; ++Itr)
	//{
	//	if (IsValidAndLoaded(Itr->GetOwner()))
	//	{
	//		if (Itr->Init())
	//		{
	//			ContainerMonitors.Emplace(*Itr);
	//			TSharedPtr<FSLContainerEventHandler> Handler = MakeShareable(new FSLContainerEventHandler());
	//			Handler->Init(*Itr);
	//			if (Handler->IsInit())
	//			{
	//				EventHandlers.Add(Handler);
	//			}
	//			else
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
	//					*FString(__func__), __LINE__, *Itr->GetName());
	//			}
	//		}

	//	}
	//}
}

// Iterate and init the pick and place monitors
void ASLSymbolicLogger::InitPickAndPlaceMonitors()
{
	for (TObjectIterator<USLPickAndPlaceMonitor> Itr; Itr; ++Itr)
	{
		if (IsValidAndLoaded(Itr->GetOwner()))
		{
			Itr->Init();
			if (Itr->IsInit())
			{
				PickAndPlaceMonitors.Emplace(*Itr);
				TSharedPtr<FSLPickAndPlaceEventsHandler> EvHandler = MakeShareable(new FSLPickAndPlaceEventsHandler());
				EvHandler->Init(*Itr);
				EvHandler->EpisodeId = LocationParameters.EpisodeId;
				if (EvHandler->IsInit())
				{
					EventHandlers.Add(EvHandler);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's handler could not be init.."),
						*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's monitor could not be init.."),
					*FString(__func__), __LINE__, *Itr->GetOwner()->GetName(), *Itr->GetName());
			}
		}
	}
}

// Iterate and init the slicing monitors
void ASLSymbolicLogger::InitSlicingMonitors()
{
#if SL_WITH_SLICING
	for (TObjectIterator<USlicingBladeComponent> Itr; Itr; ++Itr)
	{
		// Make sure the object is in the world
		if (IsValidAndLoaded(Itr->GetOwner()))
		{
			// Create a Slicing event handler 
			TSharedPtr<FSLSlicingEventHandler> SEHandler = MakeShareable(new FSLSlicingEventHandler());
			SEHandler->Init(*Itr);
			if (SEHandler->IsInit())
			{
				EventHandlers.Add(SEHandler);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d SLICING INIT %s "),
					*FString(__FUNCTION__), __LINE__, *Itr->GetFullName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Handler could not be init with parent %s.."),
					*FString(__func__), __LINE__, *Itr->GetName());
			}
		}
	}
#endif // SL_WITH_SLICING
}

// Publish data through ROS
void ASLSymbolicLogger::InitROSPublisher()
{
#if SL_WITH_ROSBRIDGE
	ROSPrologClient = NewObject<USLPrologClient>(this);
	ROSPrologClient->Init(WriterParams.ServerIp, WriterParams.ServerPort);
	FSLEntitiesManager::GetInstance()->SetPrologClient(ROSPrologClient);
#endif // SL_WITH_ROSBRIDGE
}

