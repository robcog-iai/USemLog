// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SLUtils.h"
#include "SLItem.h"

/**
 * Semantic map exporter
 */
class SEMLOG_API FSLMapExporter
{
public:
	// Constructor
	FSLMapExporter();

	// Generate and write semantic map
	void WriteSemanticMap(
		const TArray<ASLItem*>& DynamicItems,
		const TArray<ASLItem*>& StaticItems,
		const TPair<USceneComponent*, FString> CameraToUniqueName,
		const FString Path);

	// Get semantic map unique name
	FString GetUniqueName() { return UniqueName; };

private:
	// Add the doctype to the xml file
	inline void AddDoctype(rapidxml::xml_document<>* SemMapDoc);

	// Add rdf node attributes
	inline void AddRDFAttribures(rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode);

	// Import ontologies
	inline void ImportOntologies(rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode);

	// Add general definitions
	inline void AddGeneralDefinitions(rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode);

	// Add semantic map individual
	inline void AddMapIndividual(rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode);

	// Add map event individuals
	inline void AddMapEventIndividuals(
		rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode,
		const TArray<ASLItem*>& DynamicItems,
		const TArray<ASLItem*>& StaticItems,
		const TPair<USceneComponent*, FString> CameraToUniqueName);

	// Unique name of the map
	FString UniqueName;
};

