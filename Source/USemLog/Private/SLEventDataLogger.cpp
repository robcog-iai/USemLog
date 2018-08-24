// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EngineUtils.h"
#include "OwlEventsStatics.h"
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
	// Write events to file
	WriteToFile();
}

// Register for semantic contact events
void USLEventDataLogger::ListenToSemanticContactEvents()
{
	// Iterate all contact trigger components, and bind to their events publisher
	for (TObjectIterator<USLContactTrigger> Itr; Itr; ++Itr)
	{
		Itr->OnSemanticContactEvent.BindUObject(
			this, &USLEventDataLogger::OnSemanticContactEvent);
	}
}

// Called when a semantic contact is finished
void USLEventDataLogger::OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event)
{
	FinishedEvents.Add(Event);
	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] !!! [O1-O2]=[%s-%s]"),
		TEXT(__FUNCTION__), __LINE__, *Event->Obj1Id, *Event->Obj2Id);
}

// Write to file
bool USLEventDataLogger::WriteToFile()
{
	if (!EventsDoc.IsValid())
		return false;

	for (const auto& Ev : FinishedEvents)
	{
		Ev->AddToOwlDoc(EventsDoc.Get());
	}

	// Write map to file
	FString FullFilePath = FPaths::ProjectDir() +
		LogDirectory + TEXT("/Episodes/EventData_") + EpisodeId + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(EventsDoc->ToString(), *FullFilePath);
}

// Create events doc (experiment) template
TSharedPtr<FOwlEvents> USLEventDataLogger::CreateEventsDocTemplate(ESLEventsTemplate TemplateType, const FString& InDocId)
{
	// Create unique semlog id for the document
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	// Fill document with template values
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