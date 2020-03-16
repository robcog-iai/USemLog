// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * 
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

private:
	// Get class name of actor (if not known use label name if bDefaultToLabelName is true)
	static FString GetClassName(AActor* Actor, bool bDefaultToLabelName = false);
};
