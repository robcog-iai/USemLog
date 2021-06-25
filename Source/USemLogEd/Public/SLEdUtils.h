// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * Static helpers functions for the semantic logger editor module tookit
 */
class USEMLOGED_API FSLEdUtils
{
public:
	// Write the semantic map
	static void WriteSemanticMap(UWorld* World, bool bOverwrite = false);

	/* Managers */
	//// Get the semantic individual manager from the world, add new if none are available
	//static class ASLIndividualManager* GetOrCreateNewIndividualManager(UWorld* World, bool bCreateNew = true);

	//// Get the vis info manager form the world, add new one if none are available
	//static class ASLIndividualInfoManager* GetOrCreateNewVisualInfoManager(UWorld* World, bool bCreateNew = true);

	// Log id values 
	static void LogIds(UWorld* World);
	static void LogIds(const TArray<AActor*>& Actors);

	// Remove all tag keys
	static bool RemoveTagKey(UWorld* World, const FString& TagType, const FString& TagKey);
	static bool RemoveTagKey(const TArray<AActor*>& Actors, const FString& TagType, const FString& TagKey);

	// Remove all types
	static bool RemoveTagType(UWorld* World, const FString& TagType);
	static bool RemoveTagType(const TArray<AActor*>& Actors, const FString& TagType);

	// Add the semantic monitor components registered to tags to the actors
	static bool AddSemanticMonitorComponents(UWorld* World, bool bOverwrite = false);
	static bool AddSemanticMonitorComponents(const TArray<AActor*>& Actors, bool bOverwrite = false);

	// Enable overlaps on actors
	static bool EnableOverlaps(UWorld* World);
	static bool EnableOverlaps(const TArray<AActor*>& Actors);

	// Enable all materials for instanced static mesh rendering
	static void EnableAllMaterialsForInstancedStaticMesh();

};

