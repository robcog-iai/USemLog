// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "rapidxml/rapidxml_print.hpp"

/**
 * Semantic map exporter
 */
class FSLMapExporter
{
public:
	// Constructor
	FSLMapExporter();

	// Generate and write semantic map
	void WriteSemanticMap(
		const TMap<AActor*, FString>& ActToUniqueName,
		const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
		const TMap<FString, UInstancedStaticMeshComponent*>& FoliageClassNameToComponent,
		const TMap<UInstancedStaticMeshComponent*, TArray<TPair<FBodyInstance*, FString>>>& FoliageComponentToUniqueNameArray,
		const FString Path);

	// Get semantic map unique name
	FString GetUniqueName() { return MapUniqueName; };

private:
	// Add the doctype to the xml file
	inline void AddDoctype(rapidxml::xml_document<>* SemMapDoc);

	// Add rdf node attributes
	inline void AddRDFAttribures(rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode);

	// Import ontologies
	inline void AddOntologies(rapidxml::xml_document<>* SemMapDoc,
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
		const TMap<AActor*, FString>& ActToUniqueName,
		const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
		const TMap<FString, UInstancedStaticMeshComponent*>& FoliageClassNameToComponent,
		const TMap<UInstancedStaticMeshComponent*, TArray<TPair<FBodyInstance*, FString>>>& FoliageComponentToUniqueNameArray);

	// Map event individual node
	FORCEINLINE void AddMapIndividual(
		rapidxml::xml_document<>* SemMapDoc,
		rapidxml::xml_node<>* RDFNode,
		const FString& LocStr,
		const FString& QuatStr,
		const FString& ClassName,
		const FString& UniqueName
		);



	// Unique name of the map
	FString MapUniqueName;
};

