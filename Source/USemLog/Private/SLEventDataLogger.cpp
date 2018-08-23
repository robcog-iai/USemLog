// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "OwlEventsStatics.h"
#include "Ids.h"

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
void USLEventDataLogger::Init(const FString& InLogDirectory, const FString& InEpisodeId, ESLEventsTemplate TemplateType)
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
void USLEventDataLogger::OnSemanticContactEvent(FSLContactEvent* Event)
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
TSharedPtr<FOwlEvents> USLEventDataLogger::CreateEventsDocTemplate(ESLEventsTemplate TemplateType, const FString& InDocId)
{
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	if (TemplateType == ESLEventsTemplate::Default)
	{
		return FOwlEventsStatics::CreateDefaultExperiment(DocId);
	}
	else if (TemplateType == ESLEventsTemplate::IAI)
	{
		return FOwlEventsStatics::CreateUEExperiment(DocId);
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