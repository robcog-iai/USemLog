// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
//#include "Templates/OwlEventsIAI.h"

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
	EventsDoc = CreateEventsTemplate(TemplateType);
}

// Start logger
void USLEventDataLogger::Start()
{

}

// Finish logger
void USLEventDataLogger::Finish()
{

}

// Write to file
bool USLEventDataLogger::WriteToFile()
{
	return true;
	//if (!EventsDoc.IsValid())
	//	return false;

	//// Write map to file
	//FString FullFilePath = FPaths::ProjectDir() +
	//	LogDirectory + TEXT("/Episodes/EventData_") + EpisodeId + TEXT(".owl");
	//FPaths::RemoveDuplicateSlashes(FullFilePath);
	//return FFileHelper::SaveStringToFile(EventsDoc->ToString(), *FullFilePath);
}

// Create events doc template
TSharedPtr<FOwlEvents> USLEventDataLogger::CreateEventsTemplate(EEventsTemplate TemplateType)
{
	//if (TemplateType == EEventsTemplate::Default)
	//{
	//	return MakeShareable(new FOwlEvents());
	//}
	//else if (TemplateType == EEventsTemplate::IAI)
	//{
	//	return MakeShareable(new FOwlEventsIAI());
	//}
	return MakeShareable(new FOwlEvents());
}