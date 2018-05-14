// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Doc.h"
#include "SemanticMap/MapTemplateDefault.h"
#include "SemanticMap/MapTemplateIAIKitchen.h"

/**
* Semantic map template types
*/
UENUM(BlueprintType)
enum class EMapTemplateType : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAIKitchen  			UMETA(DisplayName = "IAI Kitchen"),
	IAISupermarket     		UMETA(DisplayName = "IAI Supermarket"),
};

/**
 * 
 */
class USEMLOG_API FSLSemanticMap
{
public:
	// Constructor
	FSLSemanticMap();

	// Constructor with map generation
	FSLSemanticMap(UWorld* World, 
		EMapTemplateType TemplateType = EMapTemplateType::NONE,
		const FString& InDirectory = TEXT("SemLog"));

	// Destructor
	~FSLSemanticMap();

	// Generate semantic map from world
	void Generate(UWorld* World, EMapTemplateType TemplateType = EMapTemplateType::NONE);

	// Set log directory
	void SetLogDirectory(const FString& InDirectory) { LogDirectory = InDirectory; }

	// Export semantic map to file
	bool WriteToFile(const FString& Filename = TEXT("SemanticMap"));

	// To string
	FString ToString() const;

private:
	// To document
	SLOwl::FDoc ToDoc() const;

	// Add semantic map entries
	void AddEntries(UWorld* World);

	// Add entry
	void AddEntry(UObject* Object,
		const FString& Id,
		const FString& Class,
		const TMap<UObject*, TMap<FString, FString>> ObjectsToTagsMap);

	// Create a pose entry
	SLOwl::FNode CreatePoseEntry(const FVector& InLoc, const FQuat& InQuat, const FString& InId);

	// Path for saving the semantic map
	FString LogDirectory;

	// XML Declaration
	FString Declaration;

	// Entity definitions
	SLOwl::FEntityDTD EntityDefinitions; // TODO use namespace shortcuts

	// Namespace declarations
	TArray<SLOwl::FAttribute> Namespaces;

	// Ontology imports 
	SLOwl::FNode OntologyImports;

	// Property definitions
	TArray<SLOwl::FNode> PropertyDefinitions;

	// Datatype definitions
	TArray<SLOwl::FNode> DatatypeDefinitions;

	// Class definitions
	TArray<SLOwl::FNode> ClassDefinitions;

	// Entity entries
	TArray<SLOwl::FNode> Entries;
};
