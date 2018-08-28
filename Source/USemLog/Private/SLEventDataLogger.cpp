// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"
#include "OwlExperimentStatics.h"
#include "SLContactTrigger.h"
#include "Ids.h"

// Constructor
USLEventDataLogger::USLEventDataLogger()
{
}

// Destructor
USLEventDataLogger::~USLEventDataLogger()
{
}

// Init Logger
void USLEventDataLogger::Init(const FString& InLogDirectory, const FString& InEpisodeId, ESLEventsTemplate TemplateType)
{
	LogDirectory = InLogDirectory;
	EpisodeId = InEpisodeId;
	OwlDocTemplate = TemplateType;

	// Create the document template
	ExperimentDoc = CreateEventsDocTemplate(TemplateType, InEpisodeId);
}

// Start logger
void USLEventDataLogger::Start()
{
	// Subscribe for semantic contact events
	USLEventDataLogger::ListenToSemanticContactRelatedEvents();
}

// Finish logger
void USLEventDataLogger::Finish()
{
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
	WriteToFile();
}

// Register for semantic contact events
void USLEventDataLogger::ListenToSemanticContactRelatedEvents()
{
	// Iterate all contact trigger components, and bind to their events publisher
	for (TObjectIterator<USLContactTrigger> Itr; Itr; ++Itr)
	{
		Itr->OnSemanticContactEvent.BindUObject(
			this, &USLEventDataLogger::OnSemanticContactEvent);
		Itr->OnSemanticSupportedByEvent.BindUObject(
			this, &USLEventDataLogger::OnSemanticSupportedByEvent);
	}
}

// Called when a semantic contact is finished
void USLEventDataLogger::OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event)
{
	FinishedEvents.Add(Event);
}

// Called when a semantic supported by event is finished
void USLEventDataLogger::OnSemanticSupportedByEvent(TSharedPtr<FSLSupportedByEvent> Event)
{
	FinishedEvents.Add(Event);
}

// Write to file
bool USLEventDataLogger::WriteToFile()
{
	if (!ExperimentDoc.IsValid())
		return false;

	// Write map to file
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/Episodes/EventData_") + EpisodeId + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(ExperimentDoc->ToString(), *FullFilePath);
}

// Create events doc (experiment) template
TSharedPtr<FOwlExperiment> USLEventDataLogger::CreateEventsDocTemplate(ESLEventsTemplate TemplateType, const FString& InDocId)
{
	// Create unique semlog id for the document
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	// Fill document with template values
	if (TemplateType == ESLEventsTemplate::Default)
	{
		return FOwlExperimentStatics::CreateDefaultExperiment(DocId);
	}
	else if (TemplateType == ESLEventsTemplate::IAI)
	{
		return FOwlExperimentStatics::CreateUEExperiment(DocId);
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