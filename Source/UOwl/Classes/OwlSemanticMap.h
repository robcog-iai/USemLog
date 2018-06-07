// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "OwlDoc.h"

/**
* Semantic map document in OWL
*/
struct FOwlSemanticMap : public FOwlDoc
{
public:
	// SemanticEnvironmentMap individual
	FOwlNode SemMapIndividual;

	// Map prefix (e.g. ue-def from rdf:about="&ue-def;4dfw4smiMD9ne1">)
	FString MapPrefix;

	// Map name (e.g UE-DefaultMap, from "http://knowrob.org/kb/UE-DefaultMap.owl#")
	FString MapName;

	// Map unique Id
	FString MapId;

public:
	// Default constructor
	FOwlSemanticMap() {}

	// Init constructor
	FOwlSemanticMap(const FString& InMapPrefix,
		const FString& InMapName,
		const FString& InMapId) :
		MapPrefix(InMapPrefix),
		MapName(InMapName),
		MapId(InMapId)
	{
		SetSemanticMapNode(InMapPrefix, InMapId);
		SetOntologyNode(InMapName);
	}
	
	// Create semantic map node individual
	void SetSemanticMapNode(const FString& InMapPrefix, const FString& InMapId)
	{
		const FOwlPrefixName OwlNI("owl", "NamedIndividual");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlAttributeValue SemMapInd(InMapPrefix, InMapId);

		SemMapIndividual.Name = OwlNI;
		SemMapIndividual.AddAttribute(FOwlAttribute(RdfAbout, SemMapInd));
	}

	// To string
	virtual FString ToString() override
	{
		FString DocStr = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
		DocStr += EntityDefinitions.ToString();
		FOwlNode Root(FOwlPrefixName("rdf", "RDF"), Namespaces);
		Root.AddChildNode(OntologyImports);
		Root.AddChildNodes(PropertyDefinitions);
		Root.AddChildNodes(DatatypeDefinitions);
		Root.AddChildNodes(ClassDefinitions);
		Root.AddChildNode(SemMapIndividual);
		Root.AddChildNodes(Entries);
		DocStr += Root.ToString(Indent);
		return DocStr;
	}
};
