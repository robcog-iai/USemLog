// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"

#include "SLOwlExperimentStatics.h"
#include "SLOverlapArea.h"
#include "SLGraspTrigger.h"
#include "SLGoogleCharts.h"
#include "Ids.h"

#if WITH_MC_GRASP
#include "MCFixationGrasp.h"
#endif // WITH_MC_GRASP

#include "Events/SLContactEventHandler.h"

// Constructor
USLEventLogger::USLEventLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
}

// Destructor
USLEventLogger::~USLEventLogger()
{
	if (!bIsFinished)
	{
		USLEventLogger::Finish();
	}
}

// Init Logger
void USLEventLogger::Init(const FString& InLogDirectory,
	const FString& InEpisodeId,
	ESLOwlExperimentTemplate TemplateType,
	bool bInWriteTimelines)
{
	if (!bIsInit)
	{
		LogDirectory = InLogDirectory;
		EpisodeId = InEpisodeId;
		OwlDocTemplate = TemplateType;
		bWriteTimelines = bInWriteTimelines;

		// Create the document template
		ExperimentDoc = CreateEventsDocTemplate(TemplateType, InEpisodeId);

		// TODO cache this for calling finish and start without re-iterating
		// Init all contact trigger components
		for (TObjectIterator<USLOverlapArea> Itr; Itr; ++Itr)
		{
			// Init the semantic overlap area
			Itr->Init();

			// Create a contact event handler 
			TSharedPtr<FSLContactEventHandler> ContactEventHandler = MakeShareable(new FSLContactEventHandler());
			ContactEventHandler->Init(*Itr);
			EventHandlers.Add(ContactEventHandler);

			// Create a supported-by event handler
			TSharedPtr<FSLSupportedByEventHandler> SupportedByEventHandler = MakeShareable(new FSLSupportedByEventHandler());
			SupportedByEventHandler->Init(*Itr);
			EventHandlers.Add(SupportedByEventHandler);
		}

#if WITH_MC_GRASP
		for (TObjectIterator<UMCFixationGrasp> Itr; Itr; ++Itr)
		{
			// Create a grasp event handler 
			TSharedPtr<FSLGraspEventHandler> GraspEventHandler = MakeShareable(new FSLGraspEventHandler());
			GraspEventHandler->Init(*Itr);
			EventHandlers.Add(GraspEventHandler);
		}
#endif // WITH_MC_GRASP

		// Mark as initialized
		bIsInit = true;
	}
}

// Start logger
void USLEventLogger::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Start handlers
		for (auto& EvHandler : EventHandlers)
		{
			// Subscribe for various semantic events
			EvHandler->Start();

			// Bind resulting events
			EvHandler->OnSemanticEvent.BindUObject(
				this, &USLEventLogger::OnSemanticEvent);
		}

		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLEventLogger::Finish()
{
	if (bIsStarted || bIsInit)
	{
		const float EndTime = GetWorld()->GetTimeSeconds();

		for (auto& EvHandler : EventHandlers)
		{
			EvHandler->Finish(EndTime);
		}



		// TODO2 instead of finishing external triggers, look unto unbinding the delegates

		// TODO check if the publishers arrive on time with the finished pending events from calling finish
		// make sure they arrive and then finish, call e.g. post finish
		// Init all contact trigger components
		for (TObjectIterator<USLOverlapArea> Itr; Itr; ++Itr)
		{
			Itr->Finish();
		}


		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;


		if (!ExperimentDoc.IsValid())
			return;

		// Add finished events to doc
		for (const auto& Ev : FinishedEvents)
		{
			Ev->AddToOwlDoc(ExperimentDoc.Get());
		}

		// Add stored unique timepoints to doc
		ExperimentDoc->AddTimepointIndividuals();

		// Add stored unique objects to doc
		ExperimentDoc->AddObjectIndividuals();

		// Add experiment individual to doc
		ExperimentDoc->AddExperimentIndividual();

		// Write events to file
		USLEventLogger::WriteToFile();
	}
}

//// Register for semantic contact events
//void USLEventLogger::ListenToSemanticEvents()
//{
//	//// Iterate all contact trigger components, and bind to their events publisher
//	//for (TObjectIterator<USLOverlapArea> Itr; Itr; ++Itr)
//	//{
//	//	if (Itr->SLContactPub.IsValid())
//	//	{
//	//		Itr->SLContactPub->OnSemanticContactEvent.BindUObject(
//	//			this, &USLEventLogger::OnSemanticContactEvent);
//	//	}
//
//	//	if (Itr->SLSupportedByPub.IsValid())
//	//	{
//	//		Itr->SLSupportedByPub->OnSupportedByEvent.BindUObject(
//	//			this, &USLEventLogger::OnSemanticSupportedByEvent);
//	//	}
//	//}
//
//	//// Iterate all grasp listeners
//	//for (TObjectIterator<USLGraspTrigger> Itr; Itr; ++Itr)
//	//{
//	//	if (Itr->SLGraspPub.IsValid())
//	//	{
//	//		Itr->SLGraspPub->OnSemanticGraspEvent.BindUObject(
//	//			this, &USLEventLogger::OnSemanticGraspEvent);
//	//	}
//	//}
//
//	// Bind event handlers
//	for (auto& EvHandler : EventHandlers)
//	{
//		EvHandler->OnSemanticEvent.BindUObject(
//			this, &USLEventLogger::OnSemanticEvent);
//	}
//}

// Called when a semantic event is done
void USLEventLogger::OnSemanticEvent(TSharedPtr<ISLEvent> Event)
{
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
	FinishedEvents.Add(Event);
}


//// Called when a semantic contact is finished
//void USLEventLogger::OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event)
//{
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
//	FinishedEvents.Add(Event);
//}
//
//// Called when a semantic supported by event is finished
//void USLEventLogger::OnSemanticSupportedByEvent(TSharedPtr<FSLSupportedByEvent> Event)
//{
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
//	FinishedEvents.Add(Event);
//}
//
//// Called when a semantic supported by event is finished
//void USLEventLogger::OnSemanticGraspEvent(TSharedPtr<FSLGraspEvent> Event)
//{
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
//	FinishedEvents.Add(Event);
//}

// Write to file
bool USLEventLogger::WriteToFile()
{
	// Write events timelines to file
	if (bWriteTimelines)
	{
		FSLGoogleChartsParameters Params;
		Params.bTooltips = true;
		FSLGoogleCharts::WriteTimelines(FinishedEvents, LogDirectory, EpisodeId, Params);
	}

	if (!ExperimentDoc.IsValid())
		return false;

	// Write experiment to file
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/Episodes/") + EpisodeId + TEXT("_ED.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(ExperimentDoc->ToString(), *FullFilePath);
}

// Create events doc (experiment) template
TSharedPtr<FSLOwlExperiment> USLEventLogger::CreateEventsDocTemplate(ESLOwlExperimentTemplate TemplateType, const FString& InDocId)
{
	// Create unique semlog id for the document
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	// Fill document with template values
	if (TemplateType == ESLOwlExperimentTemplate::Default)
	{
		return FSLOwlExperimentStatics::CreateDefaultExperiment(DocId);
	}
	else if (TemplateType == ESLOwlExperimentTemplate::IAI)
	{
		return FSLOwlExperimentStatics::CreateUEExperiment(DocId);
	}
	return MakeShareable(new FSLOwlExperiment());
}

// Finish the pending events at the current time
void USLEventLogger::FinishPendingEvents(const float EndTime)
{
	for (const auto& PE : StartedEvents)
	{
		PE->End = EndTime;
		FinishedEvents.Emplace(PE);
	}
	StartedEvents.Empty();
}