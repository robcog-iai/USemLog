// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
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

	// Write unique IDs
	static void WriteUniqueIds(UWorld* World, bool bOverwrite = false);

	// Write class names
	static void WriteClassNames(UWorld* World, bool bOverwrite = false);

	// Write unique visual masks
	static void WriteVisualMasks(UWorld* World, bool bOverwrite = false);

	// Remove all tag keys
	static void RemoveTagKey(UWorld* World, const FString& TagType, const FString& TagKey);

	// Remove all types
	static void RemoveTagType(UWorld* World, const FString& TagType);

	// Add the semantic monitor components registered to tags to the actors
	static void AddSemanticMonitorComponents(UWorld* World, bool bOverwrite = false);

	// Add data components to the actors containing all the information from the tags
	static void AddSemanticDataComponents(UWorld* World, bool bOverwrite = false);

	// Enable overlaps on actors
	static void EnableOverlaps(UWorld* World);

	// Enable all materials for instanced static mesh rendering
	static void EnableAllMaterialsForInstancedStaticMesh();

	// Toggle between showing the semantic data of the entities in the world
	static void ShowSemanticData(UWorld* World);

private:
	// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
	static FString GetClassName(AActor* Actor, bool bDefaultToLabelName = false);

	// Generate unique visual masks using randomization
	static void WriteRandomlyGeneratedVisualMasks(UWorld* World, bool bOverwrite = false);

	// Generate unique visual masks using incremental heuristic
	static void WriteIncrementallyGeneratedVisualMasks(UWorld* World, bool bOverwrite = false);

	/* Color helpers */
	// Get the manhattan distance between the colors
	FORCEINLINE static float ColorManhattanDistance(const FColor& C1, const FColor& C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static bool ColorEqual(const FColor& C1, const FColor& C2, uint8 Tolerance = 0)
	{
		if (Tolerance < 1) { return C1 == C2; }
		return ColorManhattanDistance(C1, C2) <= Tolerance;
	}

	// Generate a random color
	FORCEINLINE static FColor ColorRandomRGB()
	{
		return FColor((uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f), (uint8)(FMath::FRand() * 255.f));
	}
};
