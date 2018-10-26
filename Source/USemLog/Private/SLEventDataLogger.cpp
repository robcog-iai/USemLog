// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"

#include "SLOwlExperimentStatics.h"
#include "SLOverlapArea.h"
#include "SLGraspTrigger.h"
#include "SLGoogleCharts.h"
#include "Ids.h"

// Constructor
USLEventDataLogger::USLEventDataLogger()
{
	// Flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
USLEventDataLogger::~USLEventDataLogger()
{
	if (!bIsFinished)
	{
		USLEventDataLogger::Finish();
	}
}

// Init Logger
void USLEventDataLogger::Init(const FString& InLogDirectory,
	const FString& InEpisodeId,
	EOwlExperimentTemplate TemplateType,
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
			Itr->Init();
		}

		// Mark as initialized
		bIsInit = true;
	}
}

// Start logger
void USLEventDataLogger::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Subscribe for various semantic events
		USLEventDataLogger::ListenToSemanticEvents();

		// Mark as started
		bIsStarted = true;
	}
}

// Finish logger
void USLEventDataLogger::Finish()
{
	if (bIsStarted || bIsInit)
	{
		if (!ExperimentDoc.IsValid())
			return;

		// TODO check if the publishers arrive on time with the finished pending events from calling finish
		// make sure they arrive and then finish, call e.g. post finish
		// Init all contact trigger components
		for (TObjectIterator<USLOverlapArea> Itr; Itr; ++Itr)
		{
			Itr->Finish();
		}

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
		USLEventDataLogger::WriteToFile();

		// Mark finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Register for semantic contact events
void USLEventDataLogger::ListenToSemanticEvents()
{
	// Iterate all contact trigger components, and bind to their events publisher
	for (TObjectIterator<USLOverlapArea> Itr; Itr; ++Itr)
	{
		if (Itr->SLContactPub.IsValid())
		{
			Itr->SLContactPub->OnSemanticContactEvent.BindUObject(
				this, &USLEventDataLogger::OnSemanticContactEvent);
		}

		if (Itr->SLSupportedByPub.IsValid())
		{
			Itr->SLSupportedByPub->OnSupportedByEvent.BindUObject(
				this, &USLEventDataLogger::OnSemanticSupportedByEvent);
		}
	}

	// Iterate all grasp listeners
	for (TObjectIterator<USLGraspTrigger> Itr; Itr; ++Itr)
	{
		if (Itr->SLGraspPub.IsValid())
		{
			Itr->SLGraspPub->OnSemanticGraspEvent.BindUObject(
				this, &USLEventDataLogger::OnSemanticGraspEvent);
		}
	}
}

// Called when a semantic contact is finished
void USLEventDataLogger::OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event)
{
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
	FinishedEvents.Add(Event);
}

// Called when a semantic supported by event is finished
void USLEventDataLogger::OnSemanticSupportedByEvent(TSharedPtr<FSLSupportedByEvent> Event)
{
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
	FinishedEvents.Add(Event);
}

// Called when a semantic supported by event is finished
void USLEventDataLogger::OnSemanticGraspEvent(TSharedPtr<FSLGraspEvent> Event)
{
	UE_LOG(LogTemp, Error, TEXT(">> %s::%d %s"), TEXT(__FUNCTION__), __LINE__, *Event->Tooltip());
	FinishedEvents.Add(Event);
}

// Write to file
bool USLEventDataLogger::WriteToFile()
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
TSharedPtr<FOwlExperiment> USLEventDataLogger::CreateEventsDocTemplate(EOwlExperimentTemplate TemplateType, const FString& InDocId)
{
	// Create unique semlog id for the document
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	// Fill document with template values
	if (TemplateType == EOwlExperimentTemplate::Default)
	{
		return FSLOwlExperimentStatics::CreateDefaultExperiment(DocId);
	}
	else if (TemplateType == EOwlExperimentTemplate::IAI)
	{
		return FSLOwlExperimentStatics::CreateUEExperiment(DocId);
	}
	return MakeShareable(new FOwlExperiment());
}

// Finish the pending events at the current time
void USLEventDataLogger::FinishPendingEvents(const float EndTime)
{
	for (const auto& PE : PendingEvents)
	{
		PE->End = EndTime;
		FinishedEvents.Emplace(PE);
	}
	PendingEvents.Empty();
}