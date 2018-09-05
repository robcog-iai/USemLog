// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "OwlSemanticMap.h"

/**
 * Class for exporting the semantic map in an OWL format
 */
struct USEMLOG_API FSLSemanticMapWriter
{
public:
	// Default constructor
	FSLSemanticMapWriter();

	// Write semantic map to file
	bool WriteToFile(UWorld* World,
		EOwlSemanticMapTemplate TemplateType = EOwlSemanticMapTemplate::NONE,
		const FString& InDirectory = TEXT("SemLog"),
		const FString& InFilename = TEXT("SemanticMap"));

private:
	// Create semantic map template
	TSharedPtr<FOwlSemanticMap> CreateSemanticMapDocTemplate(
		EOwlSemanticMapTemplate TemplateType,
		const FString& DocId = "");

	// Add individuals to the semantic map
	void AddAllIndividuals(TSharedPtr<FOwlSemanticMap> InSemMap, UWorld* World);

	// Add object individual to the semantic map
	void AddObjectIndividual(TSharedPtr<FOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InId,
		const FString& InClass);

	// Add class definition individual
	void AddClassDefinition(TSharedPtr<FOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InClass,
		const FString& InSubClassOf = TEXT(""));

	// Add constraint individual
	void AddConstraintIndividual(TSharedPtr<FOwlSemanticMap> InSemMap,
		UPhysicsConstraintComponent* ConstraintComp,
		const FString& InId,
		const TArray<FName>& InTags);

	// Get object semantically annotated children ids (only direct children, no grandchildren etc.)
	TArray<FString> GetAllChildIds(UObject* Object);

	// Get object semantically annotated parent id (empty string if none)
	FString GetParentId(UObject* Object);
};
