// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Owl/SLOwlExperimentStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

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
	Experiment->AddEntityDefintion("log", "http://knowrob.org/kb/ameva_log.owl#");
	//Experiment->AddEntityDefintion(InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add namespaces
	//Experiment->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	//Experiment->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/ameva_log.owl#");
	Experiment->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/ameva_log.owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	Experiment->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Experiment->AddNamespaceDeclaration("xmlns", "log", "http://knowrob.org/kb/ameva_log.owl#");
	//Experiment->AddNamespaceDeclaration("xmlns", InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Set and add imports
	Experiment->CreateOntologyNode();
	Experiment->AddOntologyImport("package://knowrob/owl/knowrob.owl");

	// Add property definitions
	Experiment->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddPropertyDefinition("knowrob", "taskContext");
	Experiment->AddPropertyDefinition("knowrob", "taskSuccess");
	Experiment->AddPropertyDefinition("knowrob", "startTime");
	Experiment->AddPropertyDefinition("knowrob", "endTime");
	Experiment->AddPropertyDefinition("knowrob", "experiment");
	Experiment->AddPropertyDefinition("knowrob", "inContact");
	Experiment->AddPropertyDefinition("knowrob", "performedBy");
	Experiment->AddPropertyDefinition("knowrob", "objectActedOn");
	Experiment->AddPropertyDefinition("knowrob", "deviceUsed");
	Experiment->AddPropertyDefinition("knowrob", "outputsCreated");
	Experiment->AddPropertyDefinition("knowrob", "isSupported");
	Experiment->AddPropertyDefinition("knowrob", "isSupporting");
	Experiment->AddPropertyDefinition("knowrob", "inEpisode");
	Experiment->AddPropertyDefinition("knowrob", "subAction");
	Experiment->AddPropertyDefinition("knowrob", "performedInMap");

	// Add datatype definitions
	Experiment->AddDatatypeDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddDatatypeDefinition("knowrob", "quaternion");
	Experiment->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	Experiment->AddClassDefinition(FOwlCommentNode("Class Definitions"));
	Experiment->AddClassDefinition("knowrob", "AmevaExperiment");
	Experiment->AddClassDefinition("knowrob", "GraspingSomething");
	Experiment->AddClassDefinition("knowrob", "SlicingSomething");
	Experiment->AddClassDefinition("knowrob", "TouchingSituation");
	Experiment->AddClassDefinition("knowrob", "SupportedBySituation");
	Experiment->AddClassDefinition("knowrob", "ContainerManipulation");
	Experiment->AddClassDefinition("knowrob", "PickUpSituation");
	Experiment->AddClassDefinition("knowrob", "PreGraspSituation");
	Experiment->AddClassDefinition("knowrob", "PutDownSituation");
	Experiment->AddClassDefinition("knowrob", "ReachingForSomething");
	Experiment->AddClassDefinition("knowrob", "SlidingSituation");
	Experiment->AddClassDefinition("knowrob", "TransportingSituation");


	// Add individuals comment
	// Experiment->AddExperimentIndividual(InDocPrefix, InDocId); // Adding at end
	Experiment->AddIndividual(FOwlCommentNode("Event Individuals"));

	return Experiment;
}

//// Create UE experiment document
//TSharedPtr<FSLOwlExperiment> FSLOwlExperimentStatics::CreateUEExperiment(
//	const FString& InDocId,
//	const FString& InDocPrefix,
//	const FString& InDocOntologyName)
//{
//	TSharedPtr<FSLOwlExperiment> Experiment = FSLOwlExperimentStatics::CreateDefaultExperiment(
//		InDocId, InDocPrefix, InDocOntologyName);
//
//	Experiment->AddOntologyImport("package://knowrob/owl/knowrob_iai_kitchen_ue.owl");
//
//	return Experiment;
//}

// Write experiment to file
void FSLOwlExperimentStatics::WriteToFile(TSharedPtr<FSLOwlExperiment> Experiment, const FString& Path, bool bOverwrite)
{
	// Write owl data to file
	if (Experiment.IsValid())
	{
		// Write experiment to file
		FString FullFilePath = Path + "/" + Experiment->Id + TEXT("_ED.owl");
		FPaths::RemoveDuplicateSlashes(FullFilePath);
		if (!FPaths::FileExists(FullFilePath) || bOverwrite)
		{
			FFileHelper::SaveStringToFile(Experiment->ToString(), *FullFilePath);
		}
	}
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

// Create inEpisode property
FSLOwlNode FSLOwlExperimentStatics::CreateInEpisodeProperty(const FString& InDocPrefix, const FString& EpisodeId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "inEpisode");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, EpisodeId)));
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

// Create deviceUsed property
FSLOwlNode FSLOwlExperimentStatics::CreateDeviceUsedProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "deviceUsed");

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

// Create outputsCreated property
FSLOwlNode FSLOwlExperimentStatics::CreateOutputsCreatedProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "outputsCreated");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create taskSuccess property
FSLOwlNode FSLOwlExperimentStatics::CreateTaskSuccessProperty(const FString& InDocPrefix, const bool TaskSuccess)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "taskSuccess");

	const FString Id = TaskSuccess ? "true" : "false";
	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, Id)));
}

FSLOwlNode FSLOwlExperimentStatics::CreateGraspTypeProperty(const FString& InDocPrefix, const FString& InGraspType)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "graspType");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InGraspType)));
}

FSLOwlNode FSLOwlExperimentStatics::CreateTypeProperty(const FString& InDocPrefix, const FString& InType)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbPrefix("knowrob", "type");

	return FSLOwlNode(KbPrefix, FSLOwlAttribute(
		RdfResource, FSLOwlAttributeValue(InDocPrefix, InType)));
}
