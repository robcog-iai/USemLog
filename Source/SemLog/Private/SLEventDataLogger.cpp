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
		bIsStarted = true;
		
		// Add comment nodes
		FinishedEvents.EmplaceAt(0, MakeShareable(new FOwlNode("Event Individuals")));
		// TODO see for workaround if comments are wanted here
		//ObjectIndividuals.EmplaceAt(0, MakeShareable(new FOwlNode("Object Individuals")));

		return USLEventDataLogger::StartMetadataEvent(Timestamp);
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

		UE_LOG(LogTemp, Warning, TEXT("IDLE events nr: %i"), OpenedEvents.Num());
		UE_LOG(LogTemp, Warning, TEXT("FINISHED events nr: %i"), FinishedEvents.Num());
		for (const auto& FinishedEventsItr : FinishedEvents)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t \t Ev: %s"), *FinishedEventsItr->Object);
		}

		OwlDocument.Nodes.Append(ObjectIndividuals.Array());
		OwlDocument.Nodes.Append(TimeIndividuals.Array());
		OwlDocument.Nodes.Append(FinishedEvents);

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

// Insert finished event
bool USLEventDataLogger::InsertFinishedEvent(const TSharedPtr<FOwlNode> Event)
{
	if (bIsStarted)
	{
		FinishedEvents.Emplace(Event);
		return true;
	}
	return false;
}

// Start an event
bool USLEventDataLogger::StartAnEvent(const TSharedPtr<FOwlNode> Event)
{
	if (bIsStarted)
	{		
		// Add event to the opened events map
		OpenedEvents.Emplace(Event);
		return true;
	}
	return false;
};

// Finish an event
bool USLEventDataLogger::FinishAnEvent(const TSharedPtr<FOwlNode> Event)
{
	if (bIsStarted && OpenedEvents.Contains(Event))
	{
		// Add event to the finished ones
		FinishedEvents.Emplace(Event);
		// Remove event as opened
		OpenedEvents.Remove(Event);
		return true;
	}
	return false;
}

// Add object individual
bool USLEventDataLogger::AddObjectIndividual(TSharedPtr<FOwlNode> Object)
{
	if (bIsStarted)
	{
		ObjectIndividuals.Emplace(Object);
		return true;
	}
	return false;
}

// Add object individual
bool USLEventDataLogger::AddTimeIndividual(TSharedPtr<FOwlNode> Object)
{
	if (bIsStarted)
	{
		TimeIndividuals.Emplace(Object);
		return true;
	}
	return false;
}

// Add metadata property
bool USLEventDataLogger::AddMetadataProperty(TSharedPtr<FOwlTriple> Property)
{
	if (bIsStarted && MetaEvent.IsValid())
	{
		// Add metadata property
		MetaEvent->Properties.Emplace(*Property);
		return true;
	}
	return false;
}

// Start metadata event
bool USLEventDataLogger::StartMetadataEvent(const float Timestamp)
{
	if (bIsStarted)
	{
		MetaEvent = MakeShareable(new FOwlNode(
			"owl:NamedIndividual", "rdf:about", "&log;UnrealExperiment_" + EpisodeId,
			"Metadata Individual"));
		// Add event class
		MetaEvent->Properties.Emplace(FOwlTriple(
			"rdf:type", "rdf:resource", "&knowrob_u;UnrealExperiment"));
		// Add episode unique Id
		MetaEvent->Properties.Emplace(FOwlTriple(
			"knowrob:experiment", "rdf:datatype", "&xsd;string", EpisodeId));
		// Add event start time
		MetaEvent->Properties.Emplace(FOwlTriple(
			"knowrob:startTime",
			"rdf:resource",
			"&log;timepoint_" + FString::SanitizeFloat(Timestamp)));
		return true;
	}
	return false;
}

// Start metadata event
bool USLEventDataLogger::FinishMetadataEvent(const float Timestamp)
{
	if (bIsStarted && MetaEvent.IsValid())
	{
		// Add event end time
		MetaEvent->Properties.Emplace(FOwlTriple(
			"knowrob:startEnd",
			"rdf:resource",
			"&log;timepoint_" + FString::SanitizeFloat(Timestamp)));
		FinishedEvents.Emplace(MetaEvent);
		return true;
	}
	return false;
}

// Terminate all idling events
bool USLEventDataLogger::FinishOpenedEvents(const float Timestamp)
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Finishing OPENED EVENTS: "));
		for (auto EventItr(OpenedEvents.CreateIterator()); EventItr; ++EventItr)
		{
			// Add event end time
			(*EventItr)->Properties.Emplace(FOwlTriple(
				"knowrob:endTime",
				"rdf:resource",
				"&log;timepoint_" + FString::SanitizeFloat(Timestamp)));
			// Add event to the finished ones
			FinishedEvents.Emplace(*EventItr);
			// Remove event from the opened ones
			EventItr.RemoveCurrent();
		}
		return true;		
	}
	return false;
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
	TSharedPtr<FOwlNode> OntologyOwlNode = MakeShareable(new FOwlNode("owl:Ontology", "rdf:about", "http://knowrob.org/kb/u_map.owl",
		OntologyOwlProperties,
		"Ontologies"));
	OwlDocument.Nodes.Insert(OntologyOwlNode, 0);

	// Add object property definitions 
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;taskContext",
		"Property Definitions")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;taskSuccess")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;startTime")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;endTime")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob;experiment")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob_u;inContact")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:ObjectProperty", "rdf:about", "&knowrob_u;semanticMap")));

	// Add class definitions
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob;GraspingSomething",
		"Class Definitions")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;UnrealExperiment")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;TouchingSituation")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;KitchenEpisode")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateClosed")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateHalfClosed")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateOpened")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;FurnitureStateHalfOpened")));

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
