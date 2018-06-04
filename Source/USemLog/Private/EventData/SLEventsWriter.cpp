// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EventData/SLEventsWriter.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// UOwl
#include "OwlEventsIAI.h"

// UUtils
#include "Tags.h"
#include "Ids.h"
#include "Conversions.h"

// Default constructor
FSLEventsWriter::FSLEventsWriter()
{
}

// Write semantic map to file
bool FSLEventsWriter::WriteToFile(EEventsTemplateType TemplateType,
	const FString& InDirectory,
	const FString& InFilename)
{
	// Create the semantic map template
	TSharedPtr<FOwlEvents> EventsDoc = CreateEventsDocTemplate(TemplateType);

	// Write doc to file
	FString FullFilePath = FPaths::ProjectDir() +
		InDirectory + TEXT("/") + InFilename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(EventsDoc->ToString(), *FullFilePath);
}

// Create semantic map template
TSharedPtr<FOwlEvents> FSLEventsWriter::CreateEventsDocTemplate(EEventsTemplateType TemplateType)
{
	if (TemplateType == EEventsTemplateType::Default)
	{
		return MakeShareable(new FOwlEvents());
	}
	else if (TemplateType == EEventsTemplateType::IAI)
	{
		return MakeShareable(new FOwlEventsIAI());
	}
	return MakeShareable(new FOwlEvents());
}