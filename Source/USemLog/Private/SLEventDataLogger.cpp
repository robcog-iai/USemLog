// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "OwlEventsStatics.h"

// Constructor
USLEventDataLogger::USLEventDataLogger()
{
}

// Destructor
USLEventDataLogger::~USLEventDataLogger()
{
}

// Init Logger
void USLEventDataLogger::Init(const FString& InLogDirectory, const FString& InEpisodeId, EEventsTemplate TemplateType)
{
	LogDirectory = InLogDirectory;
	EpisodeId = InEpisodeId;
	OwlDocTemplate = TemplateType;

	// Create the document template
	EventsDoc = CreateEventsDocTemplate(TemplateType, InEpisodeId);
}

// Start logger
void USLEventDataLogger::Start()
{

}

// Finish logger
void USLEventDataLogger::Finish()
{
	// Write events to file
	WriteToFile();
}

// Write to file
bool USLEventDataLogger::WriteToFile()
{
	if (!EventsDoc.IsValid())
		return false;

	// Write map to file
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/Episodes/EventData_") + EpisodeId + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(EventsDoc->ToString(), *FullFilePath);
}

// Create events doc (experiment) template
TSharedPtr<FOwlEvents> USLEventDataLogger::CreateEventsDocTemplate(EEventsTemplate TemplateType, const FString& InExperimentId)
{
	const FString ExperimentId = InExperimentId.IsEmpty() ? FIds::NewGuidInBase64Url() : InExperimentId;

	if (TemplateType == EEventsTemplate::Default)
	{
		return FOwlEventsStatics::CreateDefaultExperiment(ExperimentId);
	}
	else if (TemplateType == EEventsTemplate::IAI)
	{
		return FOwlEventsStatics::CreateUEExperiment(ExperimentId);
	}
	return MakeShareable(new FOwlEvents());
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