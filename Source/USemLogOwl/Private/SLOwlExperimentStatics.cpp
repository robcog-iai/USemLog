// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLOwlExperimentStatics.h"

/* Semantic map template creation */
// Create default experiment document
TSharedPtr<FSLOwlExperiment> FSLOwlExperimentStatics::CreateDefaultExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	// Create map document
	TSharedPtr<FSLOwlExperiment> Experiment = MakeShareable(
		new FSLOwlExperiment(InDocPrefix, InDocOntologyName, InDocId));

	// Add definitions
	Experiment->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	Experiment->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	Experiment->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Experiment->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Experiment->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Experiment->AddEntityDefintion(InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add namespaces
	Experiment->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Experiment->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	Experiment->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Experiment->AddNamespaceDeclaration("xmlns", InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Set and add imports
	Experiment->AddOntologyNode();
	Experiment->AddOntologyImport("package://knowrob_common/owl/knowrob.owl");

	// Add property definitions
	Experiment->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddPropertyDefinition("knowrob", "taskContext");
	Experiment->AddPropertyDefinition("knowrob", "taskSuccess");

	Experiment->AddPropertyDefinition("knowrob", "startTime");
	Experiment->AddPropertyDefinition("knowrob", "endTime");
	Experiment->AddPropertyDefinition("knowrob", "experiment");
	Experiment->AddPropertyDefinition("knowrob", "inContact");

	// Add datatype definitions
	Experiment->AddDatatypeDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddDatatypeDefinition("knowrob", "quaternion");
	Experiment->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	Experiment->AddClassDefinition(FOwlCommentNode("Class Definitions"));
	Experiment->AddClassDefinition("knowrob", "UnrealExperiment");
	Experiment->AddClassDefinition("knowrob", "GraspingSomething");
	Experiment->AddClassDefinition("knowrob", "TouchingSituation");
	Experiment->AddClassDefinition("knowrob", "Pose");

	// Add individuals comment
	// Experiment->AddExperimentIndividual(InDocPrefix, InDocId); // Adding at end
	Experiment->AddIndividual(FOwlCommentNode("Event Individuals"));

	return Experiment;
}

// Create UE experiment document
TSharedPtr<FSLOwlExperiment> FSLOwlExperimentStatics::CreateUEExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	TSharedPtr<FSLOwlExperiment> Experiment = FSLOwlExperimentStatics::CreateDefaultExperiment(
		InDocId, InDocPrefix, InDocOntologyName);

	Experiment->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_kitchen_ue.owl");

	return Experiment;
}


/* Owl individuals creation */
// Create an object individual
FSLOwlNode FSLOwlExperimentStatics::CreateEventIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode Individual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}

// Create a timepoint individual
FSLOwlNode FSLOwlExperimentStatics::CreateTimepointIndividual(
	const FString& InDocPrefix,
	const float Timepoint)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	FSLOwlNode Individual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(
		InDocPrefix, Id)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty("Timepoint"));
	return Individual;
}

// Create an object individual
FSLOwlNode FSLOwlExperimentStatics::CreateObjectIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode Individual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}


/* Owl properties creation */
// Create class property
FSLOwlNode FSLOwlExperimentStatics::CreateClassProperty(const FString& InClass)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName RdfType("rdf", "type");

	return FSLOwlNode(RdfType, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue("knowrob", InClass)));
}

// Create startTime property
FSLOwlNode FSLOwlExperimentStatics::CreateStartTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "startTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, Id)));
}

// Create endTime property
FSLOwlNode FSLOwlExperimentStatics::CreateEndTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "endTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, Id)));
}

// Create inContact property
FSLOwlNode FSLOwlExperimentStatics::CreateInContactProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "inContact");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create isSupported property
FSLOwlNode FSLOwlExperimentStatics::CreateIsSupportedProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "isSupported");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create supports property
FSLOwlNode FSLOwlExperimentStatics::CreateIsSupportingProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "isSupporting");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create performedBy property
FSLOwlNode FSLOwlExperimentStatics::CreatePerformedByProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "performedBy");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create objectActedOn property
FSLOwlNode FSLOwlExperimentStatics::CreateObjectActedOnProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "objectActedOn");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}
