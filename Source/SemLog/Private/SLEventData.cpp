// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEventData.h"
#include "PlatformFilemanager.h"
#include "FileManager.h"
#include "FileHelper.h"
#include "SLStatics.h"

// Default constructor
USLEventData::USLEventData()
{
	// Default values
	bOwlDefaultValuesSet = false;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
USLEventData::~USLEventData()
{
}

// Initialise logger
bool USLEventData::Init(const FString InEpisodeId, const FString InLogDirectoryPath)
{
	LogDirectoryPath = InLogDirectoryPath;
	EpisodeId = InEpisodeId;

	USLEventData::SetDefaultValues();

	bIsInit = true;
	return true;
}

// Start logger
bool USLEventData::Start(const float Timestamp)
{
	if (!bIsInit)
	{
		return false;
	}

	USLEventData::StartMetadataEvent(Timestamp);

	bIsStarted = true;
	return true;
}

// Finish logger
bool USLEventData::Finish(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}

	USLEventData::FinishAllIdleEvents(Timestamp);
	USLEventData::FinishMetadataEvent(Timestamp);

	bIsFinished = true;
	return true;
}

// Write document to file
bool USLEventData::WriteToFile()
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

// Get document as a string
bool USLEventData::GetAsString(FString& Document)
{
	if (!bIsFinished)
	{
		return false;
	}

	// Get document as string
	Document = OwlDocument.ToXmlString();
	return true;
}

// Set document default values
void USLEventData::SetDefaultValues()
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
		"owl:imports", "rdf:resource", "package://knowrob_common/owl/knowrob_u.owl"));
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
void USLEventData::RemoveDefaultValues()
{
	// Remove default attributes
	OwlDocument.DoctypeAttributes.Empty();
	OwlDocument.RdfAttributes.Empty();

	// Mark that default values have been removed
	bOwlDefaultValuesSet = false;
}

// Start an event
bool USLEventData::StartEvent()
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
};

// Finish an event
bool USLEventData::FinishEvent()
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Insert finished event
bool USLEventData::InsertFinishedEvent()
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Start metadata event
bool USLEventData::StartMetadataEvent(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Start metadata event
bool USLEventData::FinishMetadataEvent(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}

// Terminate all idling events
bool USLEventData::FinishAllIdleEvents(const float Timestamp)
{
	if (!bIsStarted)
	{
		return false;
	}
	return true;
}
