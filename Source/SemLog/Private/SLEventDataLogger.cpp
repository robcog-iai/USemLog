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
	bFilterEvents = false;
	bConcatenateEvents = false;
	bConcatenateFirst = false;
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
		return USLEventDataLogger::StartMetadataEvent(Timestamp);
	}
	return false;
}

// Finish logger
bool USLEventDataLogger::FinishLogger(const float Timestamp)
{
	if (bIsStarted && !bIsFinished)
	{
		// Close and move all opened events to the finished ones
		USLEventDataLogger::FinishOpenedEvents(Timestamp);

		// Check to run concatenation or removal of various events
		if (bConcatenateFirst)
		{
			// Concatenate events
			if (bConcatenateEvents)
			{
				USLEventDataLogger::ConcatenateEvents();
			}

			// Filter event (remove events)
			if (bFilterEvents)
			{
				USLEventDataLogger::FilterEvents();
			}
		}
		else
		{
			// Filter event (remove events)
			if (bFilterEvents)
			{
				USLEventDataLogger::FilterEvents();
			}

			// Concatenate events
			if (bConcatenateEvents)
			{
				USLEventDataLogger::ConcatenateEvents();
			}
		}

		// Set object/time individuals, and sub-actions
		USLEventDataLogger::SetObjectsAndMetaSubActions();

		// Add events to the owl document
		OwlDocument.AppendNodes(FinishedEvents, "Event Individuals");

		// Add object individuals do the document
		TArray<TSharedPtr<FOwlNode>> GeneratedObjIndividuals;
		ObjectIndividualsMap.GenerateValueArray(GeneratedObjIndividuals);
		OwlDocument.AppendNodes(GeneratedObjIndividuals, "Object Individuals");
		
		// Add time individuals to the document
		TArray<TSharedPtr<FOwlNode>> GeneratedTimeIndividuals;
		TimeIndividualsMap.GenerateValueArray(GeneratedTimeIndividuals);
		OwlDocument.AppendNodes(GeneratedTimeIndividuals, "Time Individuals");

		// Close and move the metadata event to the finished ones
		USLEventDataLogger::FinishMetadataEvent(Timestamp);
		
		bIsStarted = false;
		bIsFinished = true;
		return true;
	}
	return false;
}

// Write document to file
bool USLEventDataLogger::WriteEventsToFile(const FString LogDirectoryPath, bool bWriteTimelines)
{
	if (!bIsFinished)
	{
		return false;
	}

	const FString Filename = "EventData_" + EpisodeId + ".owl";
	const FString FilePath = LogDirectoryPath.EndsWith("/") 
		? (LogDirectoryPath + "Episodes/EventData_" + EpisodeId + "/" + Filename) 
		: (LogDirectoryPath + "/Episodes/EventData_" + EpisodeId + "/" + Filename);

	// Return false if file already exists
	if (IFileManager::Get().FileExists(*FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("%s already exists at %s"),
			*Filename, *LogDirectoryPath);
		return false;
	}

	if (bWriteTimelines)
	{
		USLEventDataLogger::WriteTimelines(LogDirectoryPath);
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
bool USLEventDataLogger::AddObjectIndividual(const FString Id, TSharedPtr<FOwlNode> Object)
{
	if (bIsStarted)
	{
		ObjectIndividualsMap.Emplace(Id, Object);
		return true;
	}
	return false;
}

// Add time individual
bool USLEventDataLogger::AddTimeIndividual(const FString Id, TSharedPtr<FOwlNode> Object)
{
	if (bIsStarted)
	{
		TimeIndividualsMap.Emplace(Id, Object);
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

// Set filter parameters
void USLEventDataLogger::SetFilterParameters(bool bInFilterEvents, float MinDuration, bool bInFilterAll, const TArray<FString>& InFilterKeywords)
{
	bFilterEvents = bInFilterEvents;
	MinDurationFilter = MinDuration;
	bFilterAll = bInFilterAll;
	FilterKeywords = InFilterKeywords;
}

// Set concatenate events parameters
void USLEventDataLogger::SetConcatenateParameters(bool bInConcatenateEvents, float MinDuration, bool bInConcatenateFirst, bool bInConcatenateAll, const TArray<FString>& InConcatenateKeywords)
{
	bConcatenateEvents = bInConcatenateEvents;
	MinDurationConcatenate = MinDuration;
	bConcatenateFirst = bInConcatenateFirst;
	bConcatenateAll = bInConcatenateAll;
	ConcatenateKeywords = InConcatenateKeywords;
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
			"rdf:type", "rdf:resource", "&knowrob;UnrealExperiment"));
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
			"knowrob:endTime",
			"rdf:resource",
			"&log;timepoint_" + FString::SanitizeFloat(Timestamp)));
		OwlDocument.Nodes.Emplace(MetaEvent);
		return true;
	}
	return false;
}

// Terminate all opened events
bool USLEventDataLogger::FinishOpenedEvents(const float Timestamp)
{
	if (bIsStarted)
	{
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

// Filter events
void USLEventDataLogger::FilterEvents()
{
	FinishedEvents.RemoveAll([&](TSharedPtr<FOwlNode> Event) 
	{ 
		FString StartTime;
		FString EndTime;
		FString TaskContext;
		// Iterate properties and check for keywords in the subject		
		for (const auto& PropItr : Event->Properties)
		{
			if (PropItr.Subject.Contains("startTime"))
			{
				PropItr.Object.Split("_", (FString*)nullptr, &StartTime);
			}
			else if (PropItr.Subject.Contains("endTime"))
			{
				PropItr.Object.Split("_", (FString*)nullptr, &EndTime);
			}
			else if (PropItr.Subject.Contains("taskContext"))
			{
				TaskContext = PropItr.Value;
			}
		}

		if (StartTime.IsEmpty() || EndTime.IsEmpty())
		{
			return false;
		}

		if (bFilterAll)
		{
			if (FCString::Atof(*EndTime) - FCString::Atof(*StartTime) < MinDurationFilter)
			{
				//UE_LOG(LogTemp, Warning, TEXT(" *** Filtering out an event with taskContext: %s"), *TaskContext);
				return true;
			}
		}
		else
		{
			for (const auto& KeyWord : FilterKeywords)
			{
				if (TaskContext.Contains(KeyWord))
				{
					if (FCString::Atof(*EndTime) - FCString::Atof(*StartTime) < MinDurationFilter)
					{
						//UE_LOG(LogTemp, Warning, TEXT(" *** Filtering out an event with taskContext: %s"), *TaskContext);
						return true;
					}
				}
			}
		}
		return false;
	});
}

// Concatenate events
void USLEventDataLogger::ConcatenateEvents()
{
	// Iterate finished events and add them to a multi map with the task context as key
	TMap<FString, TArray<TSharedPtr<FOwlNode>>> ContextToEvents;
	for (const auto& EvItr : FinishedEvents)
	{
		// Iterate properties and check for keywords in the subject		
		for (const auto& PropItr : EvItr->Properties)
		{
			if (PropItr.Subject.Contains("taskContext"))
			{
				if (ContextToEvents.Contains(PropItr.Value))
				{
					ContextToEvents[PropItr.Value].Add(EvItr);
					break;
				}
				else
				{
					TArray<TSharedPtr<FOwlNode>> Events;
					Events.Add(EvItr);
					ContextToEvents.Add(PropItr.Value, Events);
					break;
				}
			}
		}
	}

	// Concatenate events
	for (auto& CtxToEvsItr : ContextToEvents)
	{
		// Check that we have at least 2 events with the same name
		if (CtxToEvsItr.Value.Num() > 1)
		{
			//UE_LOG(LogTemp, Warning, TEXT(" *** Context: %s, Num: %i"), *CtxToEvsItr.Key, CtxToEvsItr.Value.Num());

			 // Sort the array of the events
			CtxToEvsItr.Value.Sort([](const TSharedPtr<FOwlNode>& LHS, const TSharedPtr<FOwlNode>& RHS) -> bool	{
				// Get the end times of both events
				FString LHSEndTime;
				FString RHSEndTime;
				// Get the end time of the left hand side event		
				for (const auto& PropItr : LHS->Properties)
				{
					if (PropItr.Subject.Contains("endTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &LHSEndTime);
						break;
					}
				}
				// Get the end time of the right hand side event
				for (const auto& PropItr : RHS->Properties)
				{
					if (PropItr.Subject.Contains("endTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &RHSEndTime);
						break;
					}
				}
				return (FCString::Atof(*LHSEndTime) < FCString::Atof(*RHSEndTime));
			});

			// Iterate the sorted events backwards
			for (int32 Index = CtxToEvsItr.Value.Num() - 1; Index > 0; --Index)
			{
				// Local variables pointing to the start end time of the current and the event before
				FString CurrEvStartTime;
				FString CurrEvEndTime;
				FString BeforeEvStartTime;
				FString BeforeEvEndTime;

				// Get the start and end time of the current event
				for (const auto& PropItr : CtxToEvsItr.Value[Index]->Properties)
				{
					if (PropItr.Subject.Contains("startTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &CurrEvStartTime);
					}
					else if (PropItr.Subject.Contains("endTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &CurrEvEndTime);
					}
				}

				// Get the start and end time of the one before the current event
				for (const auto& PropItr : CtxToEvsItr.Value[Index - 1]->Properties)
				{
					if (PropItr.Subject.Contains("startTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &BeforeEvStartTime);
					}
					else if (PropItr.Subject.Contains("endTime"))
					{
						PropItr.Object.Split("_", (FString*)nullptr, &BeforeEvEndTime);
					}
				}

				// Check duration between the two events
				if (FCString::Atof(*CurrEvStartTime) - FCString::Atof(*BeforeEvEndTime) < MinDurationConcatenate)
				{
					// Remove the current event from the finished ones
					FinishedEvents.Remove(CtxToEvsItr.Value[Index]);
					// Remove the current event from the ordered array
					CtxToEvsItr.Value.RemoveAt(Index, 1, false);
					// Set the end time of the event before the current one, to the current one
					for (auto& PropItr : CtxToEvsItr.Value[Index - 1]->Properties)
					{
						if (PropItr.Subject.Contains("endTime"))
						{
							PropItr.Object.Reset();
							PropItr.Object = "&log;timepoint_" + CurrEvEndTime;
							break;
						}
					}

					//UE_LOG(LogTemp, Warning, TEXT(" \t \t ----[%s, %s]---[%s, %s]---- ==>  ----[%s, %s]---"),
					//	*BeforeEvStartTime, *BeforeEvEndTime, *CurrEvStartTime, *CurrEvEndTime,
					//	*BeforeEvStartTime, *CurrEvEndTime);
				}
			}
		}		
	}
}

// @TODO Temp solution
// Set objects, time events and metadata subActions
void USLEventDataLogger::SetObjectsAndMetaSubActions()
{
	// Iterate all closed events
	for (const auto& EvItr : FinishedEvents)
	{
		// Add event as a sub-action property in the metadata
		MetaEvent->Properties.Add(FOwlTriple("knowrob:subAction", "rdf:resource", EvItr->Object));
		
		// Iterate properties and check for keywords in the subject		
		for (const auto& PropItr : EvItr->Properties)
		{
			if (PropItr.Subject.Contains("Time"))
			{
				// Create time individual
				TimeIndividualsMap.Emplace(FOwlIndividualName(PropItr.Object).Id, MakeShareable(new FOwlNode(
					"owl:NamedIndividual", "rdf:about", PropItr.Object,
					TArray<FOwlTriple>{ FOwlTriple("rdf:type", "rdf:resource", "&knowrob;TimePoint") })));
			}
			else if (PropItr.Subject.Contains("inContact") 
				|| PropItr.Subject.Contains("objectActedOn")
				|| PropItr.Subject.Contains("performedBy"))
			{
				// Create object individual
				ObjectIndividualsMap.Emplace(FOwlIndividualName(PropItr.Object).Id,	MakeShareable(new FOwlNode(
						"owl:NamedIndividual", "rdf:about", PropItr.Object,
						TArray<FOwlTriple>{ FOwlTriple("rdf:type", "rdf:resource", "&knowrob;" + FOwlIndividualName(PropItr.Object).Class) })));
			}
		}
	}
}

// Write timelines
void USLEventDataLogger::WriteTimelines(const FString LogDirectoryPath)
{
	const FString TLFilename = "Timeline_" + EpisodeId + ".html";
	const FString TLFilePath = LogDirectoryPath.EndsWith("/")
		? (LogDirectoryPath + "Episodes/EventData_" + EpisodeId + "/" + TLFilename)
		: (LogDirectoryPath + "/Episodes/EventData_" + EpisodeId + "/" + TLFilename);
	
	FString TimelineStr =
		"<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\n"
		"\n"
		"<script type=\"text/javascript\">\n"
		"\t google.charts.load(\"current\", {packages:[\"timeline\"]});\n"
		"\t google.charts.setOnLoadCallback(drawChart);\n"
		"\t function drawChart() {\n"
		"\t\t var container = document.getElementById('event_tl');\n"
		"\t\t var chart = new google.visualization.Timeline(container);\n"
		"\t\t var dataTable = new google.visualization.DataTable();\n"
		"\n "
		"\t\t dataTable.addColumn({ type: 'string', id: 'Nr' });\n"
		"\t\t dataTable.addColumn({ type: 'string', id: 'Event' });\n"
		"\t\t dataTable.addColumn({ type: 'number', id: 'Start' });\n"
		"\t\t dataTable.addColumn({ type: 'number', id: 'End' });\n"
		"\n"
		"\t\t dataTable.addRows([\n"
		"\n";
	
	// Add metadata as first line
	{
		FString StartTime;
		FString EndTime;
		FString TaskContext;

		// Iterate properties and check for keywords in the subject		
		for (const auto& PropItr : MetaEvent->Properties)
		{
			if (PropItr.Subject.Contains("startTime"))
			{
				PropItr.Object.Split("_", (FString*)nullptr, &StartTime);
			}
			else if (PropItr.Subject.Contains("endTime"))
			{
				PropItr.Object.Split("_", (FString*)nullptr, &EndTime);
			}
			else if (PropItr.Subject.Contains("taskContext"))
			{
				TaskContext = PropItr.Value;
			}
		}

		if (!StartTime.IsEmpty() && !EndTime.IsEmpty() && !TaskContext.IsEmpty())
		{
			TimelineStr.Append(
				"\t\t [ \'" + TaskContext + "\' , \'"
				+ TaskContext + "\' , "
				+ FString::SanitizeFloat(FCString::Atof(*StartTime) * 1000) + " , "
				+ FString::SanitizeFloat(FCString::Atof(*EndTime) * 1000) + " ],\n"
			);
		}
	}

	// Iterate all closed events
	for (const auto& EvItr : FinishedEvents)
	{
		FString StartTime;
		FString EndTime;
		FString TaskContext;

		// Iterate properties and check for keywords in the subject		
		for (const auto& PropItr : EvItr->Properties)
		{
			if (PropItr.Subject.Contains("startTime"))
			{				
				PropItr.Object.Split("_", (FString*)nullptr, &StartTime);				
			}
			else if (PropItr.Subject.Contains("endTime"))
			{				
				PropItr.Object.Split("_", (FString*)nullptr, &EndTime);
			}
			else if (PropItr.Subject.Contains("taskContext"))
			{
				TaskContext = PropItr.Value;
			}
		}

		if (!StartTime.IsEmpty() && !EndTime.IsEmpty() && !TaskContext.IsEmpty())
		{
			TimelineStr.Append(
				"\t\t [ \'"	+ TaskContext + "\' , \'"
							+ TaskContext + "\' , " 
							+ FString::SanitizeFloat(FCString::Atof(*StartTime) * 1000) + " , " 
							+ FString::SanitizeFloat(FCString::Atof(*EndTime) * 1000) + " ],\n"
			);
		}

	}


	TimelineStr.Append(
		"\n"
		"\t\t]);\n"
		"\n"
		"\t\t var options = {\n"
		"\t\t\t timeline: { showRowLabels: false }\n"
		"\t\t };\n"
		"\n"
		"\t\t chart.draw(dataTable, options);\n"
		"\t}\n"
		"</script>\n"
		"<div id=\"event_tl\" style=\"height:900px;\"></div>"
	);
	// Creates directory tree as well
	FFileHelper::SaveStringToFile(TimelineStr, *TLFilePath);
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
	OwlDocument.DoctypeAttributes.Add("rdfs", "http://www.w3.org/2000/01/rdf-schema");
	OwlDocument.DoctypeAttributes.Add("owl", "http://www.w3.org/2002/07/owl");
	OwlDocument.DoctypeAttributes.Add("xsd", "http://www.w3.org/2001/XMLSchema#");
	OwlDocument.DoctypeAttributes.Add("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	OwlDocument.DoctypeAttributes.Add("knowrob_u", "http://knowrob.org/kb/knowrob_u.owl#");
	OwlDocument.DoctypeAttributes.Add("log", "http://knowrob.org/kb/unreal_log.owl#");
	OwlDocument.DoctypeAttributes.Add("u-map", "http://knowrob.org/kb/u_map.owl#");

	// Default rdf attributes for the semantic map
	OwlDocument.RdfAttributes.Add("xmlns:computable", "http://knowrob.org/kb/computable.owl#");
	OwlDocument.RdfAttributes.Add("xmlns:swrl", "http://www.w3.org/2003/11/swrl#");
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
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;HandManipulation")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;GraspingMulitpleItems")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;AddTopping")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;CutItem")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;CreateEntity")));
	OwlDocument.Nodes.Emplace(MakeShareable(new FOwlNode("owl:Class", "rdf:about", "&knowrob_u;DestroyEntity")));

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
