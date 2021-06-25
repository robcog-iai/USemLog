// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Owl/SLOwlDoc.h"

// Forward declarations
class USLBaseIndividual;
class ASLManager;
class UWorld;
struct FSLOwlDoc;
struct FSLOwlPrefixName;

/**
* Semantic map template types
*/
UENUM(BlueprintType)
enum class ESLOwlOntologyTemplateTypes : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	Ameva					UMETA(DisplayName = "Ameva")
};


/**
 * Helpers for writing an owl document
 */
struct USEMLOG_API FSLOwlOntologyDocUtils
{
public:
	// Steps for creating and printing the semantic map document with default arguments
	static bool CreateAndPrintDoc(UWorld* World, bool bOverwrite, ESLOwlOntologyTemplateTypes Type = ESLOwlOntologyTemplateTypes::Ameva);

	// Print the document to file
	static bool PrintDoc(const FSLOwlDoc& InDoc, const FString& Path, const FString& Filename, bool bOverwrite);

private:
	// Create a semantic map document template
	static FSLOwlDoc GetDocumentTemplate(ESLOwlOntologyTemplateTypes Type);

	// Add map boilerplate entity definitions depending on the map template
	static void AddEntityDefinitions(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type);

	// Add map boilerplate namespace declarations depending on the map template
	static void AddNamespaceDeclarations(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type);

	// Add map ontology imports depending on the map template
	static void AddOntologies(FSLOwlDoc& Doc, ESLOwlOntologyTemplateTypes Type);

	// Add the semantic map as an individual to the document
	static void AddMapIndividual(FSLOwlDoc& Doc, const FString& MapId);

	// Add the individual entities to the document
	static bool AddIndividual(FSLOwlDoc& Doc, USLBaseIndividual* Individual);

	// Add class definition if it does not exist
	static void AddUniqueClassDefinition(FSLOwlDoc& Doc, USLBaseIndividual* Individual);

	// Add class definition properties
	static void AddClassDefinitionProperties(FSLOwlNode& ClassNode, USLBaseIndividual* Individual);

	// Add bounding box properties to the class node defintion
	static void AddBBClassDefinitionProperties(FSLOwlNode& ClassNode, FVector BBSize);

	/* Common structures */
	// Owl
	static const FSLOwlPrefixName OwlNamedIndividual;
	static const FSLOwlPrefixName OwlClass;
	static const FSLOwlPrefixName OwlRestriction;
	static const FSLOwlPrefixName OwlHasValue;
	static const FSLOwlPrefixName OwlOnProperty;
	// Rdf
	static const FSLOwlPrefixName RdfAbout;
	static const FSLOwlPrefixName RdfType;
	static const FSLOwlPrefixName RdfDatatype;
	static const FSLOwlPrefixName RdfResource;
	// Rdfs
	static const FSLOwlPrefixName RdfsSubClassOf;
	// AV
	static const FSLOwlAttributeValue AVFloat;

	/* Constants */
	//constexpr static char AmevaNs[] = "ameva";
	static constexpr auto AmevaNs = TEXT("ameva_log");
	//constexpr static char KRNs[] = "knowrob";
	static constexpr auto KRNs = TEXT("knowrob");
};
