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

// 
void USLEventDataLogger::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);

}

//
void USLEventDataLogger::FinishDestroy()
{
	Super::FinishDestroy();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Init Logger
void USLEventDataLogger::Init(const FString& InLogDirectory, const FString& InEpisodeId, EEventsTemplate TemplateType)
{
	LogDirectory = InLogDirectory;
	EpisodeId = InEpisodeId;
	OwlDocTemplate = TemplateType;

	// Subscribe for semantic contact events
	USLEventDataLogger::ListenToSemanticContactEvents();

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
	USLContactPoolSingleton::GetInstance()->FinishPendingContactEvents();
	UE_LOG(LogTemp, Error, TEXT("[%s][%d] -- Get Pending Contacts -- "), TEXT(__FUNCTION__), __LINE__);
	// Write events to file
	WriteToFile();
}

// Register for semantic contact events
void USLEventDataLogger::ListenToSemanticContactEvents()
{
	USLContactPoolSingleton::GetInstance()->OnSemanticContactEvent.BindUObject(
		this, &USLEventDataLogger::OnSemanticContactEvent);
}

// Called when a semantic contact is finished
void USLEventDataLogger::OnSemanticContactEvent(IEvent* Event)
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
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