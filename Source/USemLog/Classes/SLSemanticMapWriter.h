// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SLOwlSemanticMap.h"

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
		ESLOwlSemanticMapTemplate TemplateType = ESLOwlSemanticMapTemplate::NONE,
		const FString& InDirectory = TEXT("SemLog"),
		const FString& InFilename = TEXT("SemanticMap"));

private:
	// Create semantic map template
	TSharedPtr<FSLOwlSemanticMap> CreateSemanticMapDocTemplate(
		ESLOwlSemanticMapTemplate TemplateType,
		const FString& DocId = "");

	// Add individuals to the semantic map
	void AddAllIndividuals(TSharedPtr<FSLOwlSemanticMap> InSemMap, UWorld* World);

	// Add object individual to the semantic map
	void AddObjectIndividual(TSharedPtr<FSLOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InId,
		const FString& InClass);

	// Add class definition individual
	void AddClassDefinition(TSharedPtr<FSLOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InClass,
		const FString& InSubClassOf = TEXT(""));

	// Add constraint individual
	void AddConstraintIndividual(TSharedPtr<FSLOwlSemanticMap> InSemMap,
		UPhysicsConstraintComponent* ConstraintComp,
		const FString& InId,
		const TArray<FName>& InTags);

	// Get object semantically annotated parent id (empty string if none)
	FString GetParentId(UObject* Object);

	// Get object semantically annotated children ids (only direct children, no grandchildren etc.)
	void GetChildIds(UObject* Object, TArray<FString>& OutChildIds);

	// Get mobility property
	FString GetMobility(UObject* Object);
};
