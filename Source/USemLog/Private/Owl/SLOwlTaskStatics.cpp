// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "Owl/SLOwlTaskStatics.h"

/* Semantic map template creation */
// Create default Task document
TSharedPtr<FSLOwlTask> FSLOwlTaskStatics::CreateDefaultTask(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	// Create map document
	TSharedPtr<FSLOwlTask> Task = MakeShareable(
		new FSLOwlTask(InDocPrefix, InDocOntologyName, InDocId));

	// Add definitions
	Task->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	Task->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	Task->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Task->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Task->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Task->AddEntityDefintion("log", "http://knowrob.org/kb/ameva_log.owl#");
	//Task->AddEntityDefintion(InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add namespaces
	//Task->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	//Task->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Task->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/ameva_log.owl#");
	Task->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/ameva_log.owl#");
	Task->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	Task->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	Task->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Task->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Task->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Task->AddNamespaceDeclaration("xmlns", "log", "http://knowrob.org/kb/ameva_log.owl#");
	//Task->AddNamespaceDeclaration("xmlns", InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Set and add imports
	Task->CreateOntologyNode();
	Task->AddOntologyImport("package://knowrob/owl/knowrob.owl");

	// Add property definitions
	Task->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	Task->AddPropertyDefinition("knowrob", "performedInMap");
	Task->AddPropertyDefinition("knowrob", "taskDescription");

	return Task;
}

// Write experiment to file
void FSLOwlTaskStatics::WriteToFile(TSharedPtr<FSLOwlTask> Task, const FString& Path, bool bOverwrite)
{
	// Write owl data to file
	if (Task.IsValid())
	{
		// Write experiment to file
		FString FullFilePath = Path + "/" + Task->Id + TEXT("_T.owl");
		FPaths::RemoveDuplicateSlashes(FullFilePath);
		if (!FPaths::FileExists(FullFilePath) || bOverwrite)
		{
			FFileHelper::SaveStringToFile(Task->ToString(), *FullFilePath);
		}
	}
}