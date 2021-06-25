// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/SLOwlDoc.h"

/**
* Task description document in OWL
*/
struct FSLOwlTask : public FSLOwlDoc
{
protected:
	// Task individual
	FSLOwlNode TaskIndividual;

public:
	// Default constructor
	//FSLOwlTask() {}

	// Init constructor
	FSLOwlTask(const FString& InDocPrefix,
		const FString& InDocOntologyName,
		const FString& InDocId) :
		FSLOwlDoc(InDocPrefix, InDocOntologyName, InDocId)
	{}

	// Destructor
	~FSLOwlTask() {}

	// Create semantic map node individual
	void AddTaskIndividual(const FString& InDescription, const FString& InSemMapId)
	{
		const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FSLOwlPrefixName RdfAbout("rdf", "about");
		const FSLOwlPrefixName RdfType("rdf", "type");
		const FSLOwlPrefixName RdfResource("rdf", "resource");
		const FSLOwlPrefixName KbTaskDescription("knowrob", "taskDescription");
		const FSLOwlPrefixName KrPerformedInMap("knowrob", "performedInMap");
		const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
		const FSLOwlAttributeValue AttrValString("xsd", "string");
		const FSLOwlAttributeValue TaskId(Prefix, Id);

		// Create semantic map individual
		TaskIndividual.Name = OwlNI;
		TaskIndividual.AddAttribute(FSLOwlAttribute(RdfAbout, TaskId));
		TaskIndividual.AddChildNode(FSLOwlNode(RdfType, FSLOwlAttribute(
			RdfResource, FSLOwlAttributeValue("knowrob", "Task"))));
		TaskIndividual.AddChildNode(FSLOwlNode(KbTaskDescription,
			FSLOwlAttribute(RdfDatatype, AttrValString), InDescription));
		TaskIndividual.AddChildNode(FSLOwlNode(KrPerformedInMap,
			FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(Prefix, InSemMapId))));

		TaskIndividual.Comment = "Task " + Id;

		// Add map to the document individuals
		Individuals.Add(TaskIndividual);
	}
};
