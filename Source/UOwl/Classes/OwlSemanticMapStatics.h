// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "EngineMinimal.h"
#include "OwlSemanticMap.h"

/**
* Helper functions for generating semantic maps
*/
struct UOWL_API FOwlSemanticMapStatics
{
	/* Semantic map template creation */
	// Create Default semantic map
	static TSharedPtr<FOwlSemanticMap> CreateDefaultSemanticMap(
		const FString& InMapId,
		const FString& InMapPrefix = "ue-def",
		const FString& InMapName = "UE-DefaultMap");

	// Create IAI Kitchen semantic map
	static TSharedPtr<FOwlSemanticMap> CreateIAIKitchenSemanticMap(
		const FString& InMapId,
		const FString& InMapPrefix = "ue-iai-kitchen",
		const FString& InMapName = "UE-IAI-Kitchen");

	// Create IAI Supermarket semantic map
	static TSharedPtr<FOwlSemanticMap> CreateIAISupermarketSemanticMap(
		const FString& InMapId,
		const FString& InMapPrefix = "ue-iai-supermarket",
		const FString& InMapName = "UE-IAI-Supermarket");
};
