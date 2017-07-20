// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventDataLogger.h"
#include "PlatformFilemanager.h"
#include "FileManager.h"
#include "FileHelper.h"
#include "SLUtils.h"

// Default constructor
USLEventDataLogger::USLEventDataLogger()
{
	// Default values
	bOwlDefaultValuesSet = false;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
USLEventDataLogger::~USLEventDataLogger()
{
}

// Initialize logger
bool USLEventDataLogger::InitLogger(const FString InEpisodeId)
{
	if (!bIsInit)
	{
		EpisodeId = InEpisodeId;

		USLEventDataLogger::SetDefaultValues();

		bIsInit = true;
		return true;
	}
	return false;
}

// Start logger
bool USLEventDataLogger::StartLogger(const float Timestamp)
{
	if (bIsInit && !bIsStarted)
	{
		USLEventDataLogger::StartMetadataEvent(Timestamp);

		bIsStarted = true;
		return true;
	}
	return false;
}

// Finish logger
bool USLEventDataLogger::FinishLogger(const float Timestamp)
{
	if (bIsStarted && !bIsFinished)
	{
		USLEventDataLogger::FinishOpenedEvents(Timestamp);
		USLEventDataLogger::FinishMetadataEvent(Timestamp);

		UE_LOG(LogTemp, Warning, TEXT("IDLE events nr: %i"), NameToOpenedEvent.Num());
		UE_LOG(LogTemp, Warning, TEXT("FINISHED events nr: %i"), FinishedEvents.Num());
		for (const auto& FinishedEventsItr : FinishedEvents)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t \t Ev: %s"), *FinishedEventsItr.AttributeValue);
		}

		bIsStarted = false;
		bIsFinished = true;
		return true;
	}
	return false;
}

// Write document to file
bool USLEventDataLogger::WriteEventsToFile(const FString LogDirectoryPath)
{
	if (!bIsFinished)
	{
		return false;
	}

	const FString Filename = "EventData_" + EpisodeId + ".owl";
	const FString FilePath = LogDirectoryPath.EndsWith("/") ?
		(LogDirectoryPath + "Episodes/" + Filename) : (LogDirectoryPath + "/Episodes/" + Filename);

	// Return false if file already exists
	if (IFileManager::Get().FileExists(*FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("%s already exists at %s"),
			*Filename, *LogDirectoryPath);
		return false;
	}

	// Creates directory tree as well
	return FFileHelper::SaveStringToFile(OwlDocument.ToXmlString(), *FilePath);
}

// Broadcast document
bool USLEventDataLogger::BroadcastFinishedEvents()
{
	if (!bIsFinished)
	{
		return false;
	}

	OnEventsFinished.Broadcast(OwlDocument.ToXmlString());
	return true;
}

// Get the events document as a string
bool USLEventDataLogger::GetEventsAsString(FString& OutStringDocument)
{
	if (!bIsFinished)
	{
		return false;
	}

	// Get document as string
	OutStringDocument = OwlDocument.ToXmlString();
	return true;
}

// Set document default values
void USLEventDataLogger::SetDefaultValues()
{
	if (bOwlDefaultValuesSet)
	{
		// Default values already set
		return;
	}

	// Remove previous default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();

	// Default doctype attributes for the semantic map
	OwlDocument.DoctypeAttributes.Add("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns");
	OwlDocument.DoctypeAttributes.Add("rdfs", "http://www.w3.org/2002/07/owl");
	OwlDocument.DoctypeAttributes.Add("owl", "http://www.w3.org/2001/XMLSchema#");
	OwlDocument.DoctypeAttributes.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.DoctypeAttributes.Add("knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.DoctypeAttributes.Add("log", "http://knowrob.org/kb/unreal_log.owl#");
	OwlDocument.DoctypeAttributes.Add("u-map", "http://knowrob.org/kb/u_map.owl#");

	// Default rdf attributes for the semantic map
	OwlDocument.RdfAttributes.Add("xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	OwlDocument.RdfAttributes.Add("xmlns:rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	OwlDocument.RdfAttributes.Add("xmlns:owl", "http://www.w3.org/2002/07/owl#");
	OwlDocument.RdfAttributes.Add("xmlns:knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:u-map", "http://knowrob.org/kb/u_map.owl#");
	OwlDocument.RdfAttributes.Add("xml:base", "http://knowrob.org/kb/u_map.owl#");

	// Create and insert the ontologies node at the beginning
	TArray<FOwlTriple> OntologyOwlProperties;
	OntologyOwlProperties.Emplace(FOwlTriple(
		"owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob.owl"));
	OntologyOwlProperties.Emplace(FOwlTriple(
		"owl:imports", "rdf:resource", "package://knowrob_robcog/owl/knowrob_u.owl"));
	FOwlNode OntologyOwlNode("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl",
		OntologyOwlProperties,
		"Ontologies");
	OwlDocument.Nodes.Insert(OntologyOwlNode, 0);

	// Add object property definitions 
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;taskContext",
		"Property Definitions"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;taskSuccess"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;startTime"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;endTime"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;experiment"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob_u;inContact"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob_u;semanticMap"));

	// Add class definitions
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob;GraspingSomething",
		"Class Definitions"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob_u;UnrealExperiment"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob_u;TouchingSituation"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob_u;KitchenEpisode"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class",	"rdf:about", "&knowrob_u;FurnitureStateClosed"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class",	"rdf:about", "&knowrob_u;FurnitureStateHalfClosed"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class",	"rdf:about", "&knowrob_u;FurnitureStateOpened"));
	OwlDocument.Nodes.Emplace(FOwlNode("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateHalfOpened"));

	// Mark that default values have been set
	bOwlDefaultValuesSet = true;
}

// Remove document default values
void USLEventDataLogger::RemoveDefaultValues()
{
	// Remove default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();

	// Mark that default values have been removed
	bOwlDefaultValuesSet = false;
}

// Insert finished event
bool USLEventDataLogger::InsertFinishedEvent(
	const TSharedPtr<FOwlIndividualName> EventIndividualName,
	const float StartTime,
	const float EndTime,
	const TArray<FOwlTriple>& Properties)
{
	if (bIsStarted)
	{
		FinishedEvents.Emplace(
			FOwlNode("owl:NamedIndividual", "rdf:about", EventIndividualName->GetFullName(), Properties));
		UE_LOG(LogTemp, Warning, TEXT("%s : EventIndividualName: %s"),
			*FString(__FUNCTION__), *EventIndividualName->GetFullName());
		return true;
	}
	return false;
}

// Start an event
bool USLEventDataLogger::StartAnEvent(
	const TSharedPtr<FOwlIndividualName> EventIndividualName,
	const float StartTime,
	const TArray<FOwlTriple>& Properties)
{
	if (bIsStarted)
	{
		//// Add start time to the properties
		//Properties.Emplace(FOwlTriple(
		//	"knowrob:StartTime",
		//	"rdf:resource",
		//	"&log;timepoint_" + FString::SanitizeFloat(StartTime)));
		
		// Add event to the opened events map
		NameToOpenedEvent.Emplace(
			EventIndividualName,
			FOwlNode("owl:NamedIndividual", "rdf:about", EventIndividualName->GetFullName(),
			Properties));	
		UE_LOG(LogTemp, Warning, TEXT("%s : EventIndividualName: %s"),
			*FString(__FUNCTION__), *EventIndividualName->GetFullName());
		return true;
	}
	return false;
};

// Finish an event
bool USLEventDataLogger::FinishAnEvent(
	const TSharedPtr<FOwlIndividualName> EventIndividualName,
	const float EndTime,
	const TArray<FOwlTriple>& Properties)
{
	if (bIsStarted && NameToOpenedEvent.Contains(EventIndividualName))
	{
		FOwlNode ToBeFinishedEvent;
		NameToOpenedEvent.RemoveAndCopyValue(EventIndividualName, ToBeFinishedEvent);
		ToBeFinishedEvent.Properties.Append(Properties);
		FinishedEvents.Emplace(ToBeFinishedEvent);

		UE_LOG(LogTemp, Warning, TEXT("%s : EventIndividualName: %s"),
			*FString(__FUNCTION__), *EventIndividualName->GetFullName());
		return true;
	}
	return false;
}


// Start metadata event
bool USLEventDataLogger::StartMetadataEvent(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Start metadata event
bool USLEventDataLogger::FinishMetadataEvent(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Terminate all idling events
bool USLEventDataLogger::FinishOpenedEvents(const float Timestamp)
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT(" FINSIHEING OPENED EVENTS"));
		for (auto MapItr(NameToOpenedEvent.CreateIterator()); MapItr; ++MapItr)
		{	
			FinishedEvents.Emplace((*MapItr).Value);
			MapItr.RemoveCurrent();
			//(*MapItr).Value.Properties.Emplace(FOwlTriple;
			//UE_LOG(LogTemp, Warning, TEXT("%s : EventIndividualName: %s"),
			//	*FString(__FUNCTION__), *(*MapItr).Key->GetFullName());
		}
		return true;		
	}
	return false;
}
