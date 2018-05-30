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
struct USEMLOG_API FSLSemanticMapWriter
{
public:
	// Write semantic map to file
	static bool WriteToFile(UWorld* World,
		EMapTemplateType TemplateType = EMapTemplateType::NONE,
		const FString& InDirectory = TEXT("SemLog"),
		const FString& InFilename = TEXT("SemanticMap"));

private:
	// Create semantic map template
	static TSharedPtr<FOwlSemanticMap> CreateSemanticMapTemplate(EMapTemplateType TemplateType);

	// Add entries to the semantic map
	static void AddAllEntries(TSharedPtr<FOwlSemanticMap> InSemMap, UWorld* World);

	// Add object entry to the semantic map
	static void AddObjectEntry(TSharedPtr<FOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InId,
		const FString& InClass);

	// Add class definition entry
	static void AddClassDefinition(TSharedPtr<FOwlSemanticMap> InSemMap,
		UObject* Object,
		const FString& InClass,
		const FString& InParentClass = TEXT(""));

	// Add constraint entry
	static void AddConstraintEntry(TSharedPtr<FOwlSemanticMap> InSemMap,
		UPhysicsConstraintComponent* ConstraintComp,
		const FString& InId);

	// Get object semantically annotated children ids (only direct children, no grandchildren etc.)
	static TArray<FString> GetAllChildIds(UObject* Object);

	// Get object semantically annotated parent id (empty string if none)
	static FString GetParentId(UObject* Object);
};
