// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Owl.h"

/**
* Semantic map template types
*/
UENUM(BlueprintType)
enum class EMapTemplate : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAIKitchen				UMETA(DisplayName = "IAI Kitchen"),
	IAISupermarket			UMETA(DisplayName = "IAI Supermarket"),
};

/**
 * 
 */
struct USEMLOG_API FSLSemanticMapWriter
{
public:
	// Default constructor
	FSLSemanticMapWriter();

	// Write semantic map to file
	bool WriteToFile(UWorld* World,
		EMapTemplate TemplateType = EMapTemplate::NONE,
		const FString& InDirectory = TEXT("SemLog"),
		const FString& InFilename = TEXT("SemanticMap"));

private:
	// Create semantic map template
	TSharedPtr<FOwlSemanticMap> CreateSemanticMapDocTemplate(
		EMapTemplate TemplateType,
		const FString& MapId = "");

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
