// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "SLOwlExperimentStatics.h"

/* Semantic map template creation */
// Create default experiment document
TSharedPtr<FOwlExperiment> FSLOwlExperimentStatics::CreateDefaultExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	// Create map document
	TSharedPtr<FOwlExperiment> Experiment = MakeShareable(
		new FOwlExperiment(InDocPrefix, InDocOntologyName, InDocId));

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
TSharedPtr<FOwlExperiment> FSLOwlExperimentStatics::CreateUEExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	TSharedPtr<FOwlExperiment> Experiment = FSLOwlExperimentStatics::CreateDefaultExperiment(
		InDocId, InDocPrefix, InDocOntologyName);

	Experiment->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_kitchen_ue.owl");

	return Experiment;
}


/* Owl individuals creation */
// Create an object individual
FOwlNode FSLOwlExperimentStatics::CreateEventIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}

// Create a timepoint individual
FOwlNode FSLOwlExperimentStatics::CreateTimepointIndividual(
	const FString& InDocPrefix,
	const float Timepoint)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, Id)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty("Timepoint"));
	return Individual;
}

// Create an object individual
FOwlNode FSLOwlExperimentStatics::CreateObjectIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FSLOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}


/* Owl properties creation */
// Create class property
FOwlNode FSLOwlExperimentStatics::CreateClassProperty(const FString& InClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfType("rdf", "type");

	return FOwlNode(RdfType, FOwlAttribute(
		RdfResource, FOwlAttributeValue("knowrob", InClass)));
}

// Create startTime property
FOwlNode FSLOwlExperimentStatics::CreateStartTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "startTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, Id)));
}

// Create endTime property
FOwlNode FSLOwlExperimentStatics::CreateEndTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "endTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, Id)));
}

// Create inContact property
FOwlNode FSLOwlExperimentStatics::CreateInContactProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "inContact");

	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create isSupported property
FOwlNode FSLOwlExperimentStatics::CreateIsSupportedProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "isSupported");

	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create supports property
FOwlNode FSLOwlExperimentStatics::CreateIsSupportingProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "isSupporting");

	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create performedBy property
FOwlNode FSLOwlExperimentStatics::CreatePerformedByProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "performedBy");

	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create objectActedOn property
FOwlNode FSLOwlExperimentStatics::CreateObjectActedOnProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbPrefix("knowrob", "objectActedOn");

	return FOwlNode(KbPrefix, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}
