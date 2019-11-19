// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "USemLog.h"
#include "SLSemanticMapWriter.h"

/**
 * Helper functions of the editor functionalities
 */
class FSLEditorToolkit
{
public:
	// Ctor
	FSLEditorToolkit();

	// Dtor
	~FSLEditorToolkit();

	// Write the semantic map
	static void WriteSemanticMap(UWorld* World, const FString& TaskId,
		const FString& Filename = "SemanticMap",
		ESLOwlSemanticMapTemplate Template = ESLOwlSemanticMapTemplate::IAIKitchen);

	// Clear tags (all if TagType is empty)
	static void ClearTags(UWorld* World, const FString& TagType = "", const FString& KeyType = "");
	
	// Write class properties
	static void WriteClassProperties(UWorld* World, bool bOverwrite);

	// Write unique id properties
	static void WriteUniqueIdProperties(UWorld* World, bool bOverwrite);

	// Write unique mask properties
	static void WriteUniqueMaskProperties(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance, bool bRandomGenerator);
	
private:
	// Randomly generate unique visual masks
	static void RandomlyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance);

	// Incrementally generate unique visual masks
	static void IncrementallyGenerateVisualMasks(UWorld* World, bool bOverwrite, int32 VisualMaskColorMinDistance);
	
	// Get the class name of the actor
	static FString GetClassName(AActor* Actor, bool bDefaultToLabel = false);
	
	// Write to editor counterpart
	static bool WriteKVToEditorCounterpart(AActor* Actor, const FString& TagType, const FString& TagKey, const FString& TagValue, bool bReplaceExisting);

	/* Helpers */
	// Get the distance between the colors
	FORCEINLINE static float ColorDistance(const FColor& C1, const FColor C2)
	{
		return FMath::Abs(C1.R - C2.R) + FMath::Abs(C1.G - C2.G) + FMath::Abs(C1.B - C2.B);
	}

	// Check if the two colors are equal with a tolerance
	FORCEINLINE static float ColorEqual(const FColor& C1, const FColor C2, int32 Tolerance)
	{
		if(Tolerance < 1)
		{
			return C1 == C2;
		}
		else
		{
			return ColorDistance(C1, C2) <= Tolerance;
		}
	}

	// Make a random rgb color
	FORCEINLINE static FColor ColorRandomRGB()
	{
		return FColor((uint8)(FMath::FRand()*255.f), (uint8)(FMath::FRand()*255.f), (uint8)(FMath::FRand()*255.f));
	}
};

