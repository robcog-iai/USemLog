// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Doc.h"
#include "Owl/SemanticMap.h"

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
class USEMLOG_API FSLSemanticMapWriter
{
public:
	// Constructor
	FSLSemanticMapWriter();

	// Constructor with map generation
	FSLSemanticMapWriter(UWorld* World, 
		EMapTemplateType TemplateType = EMapTemplateType::NONE,
		const FString& InDirectory = TEXT("SemLog"));

	// Destructor
	~FSLSemanticMapWriter();

	// Generate semantic map from world
	void Generate(UWorld* World, EMapTemplateType TemplateType = EMapTemplateType::NONE);

	// Set log directory
	void SetLogDirectory(const FString& InDirectory) { LogDirectory = InDirectory; }

	// Export semantic map to file
	bool WriteToFile(const FString& Filename = TEXT("SemanticMap"));

private:
	// Add semantic map entries
	void AddEntries(UWorld* World);

	// Add entry
	void AddObjectEntry(UObject* Object,
		const FString& InId,
		const FString& InClass);

	// Create an object individual
	SLOwl::FNode CreateObjectIndividual(const FString& Id, const FString& Class);

	// Create class property
	SLOwl::FNode CreateClassProperty(const FString& InClass);

	// Create pose property
	SLOwl::FNode CreatePoseProperty(const FString& InId);

	// Create a pose individual
	SLOwl::FNode CreatePoseIndividual(const FString& InId, const FVector& InLoc, const FQuat& InQuat);

	// Create a location property
	SLOwl::FNode CreateLocationProperty(const FVector& InLoc);

	// Create a quaternion property
	SLOwl::FNode CreateQuaternionProperty(const FQuat& InQuat);

	// Path for saving the semantic map
	FString LogDirectory;

	// Semantic map pointer
	TSharedPtr<SLOwl::FSemanticMap> SemMap;
};
