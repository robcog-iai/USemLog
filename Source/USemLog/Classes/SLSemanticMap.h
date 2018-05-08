// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Owl/Doc.h"

/**
 * 
 */
class USEMLOG_API FSLSemanticMap
{
public:
	// Constructor
	FSLSemanticMap();

	// Constructor with map generation
	FSLSemanticMap(UWorld* World, const FString InDirectory = TEXT("SemLog"));

	// Destructor
	~FSLSemanticMap();

	// Generate semantic map from world
	void Generate(UWorld* World);

	// Export semantic map to file
	bool WriteToFile(const FString& Filename = TEXT("SemanticMap") );

private:
	// Path for saving the semantic map
	FString LogDirectory;

	// Semantic map as owl document
	SLOwl::FDoc SemMap;
};
