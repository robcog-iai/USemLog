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
	FSLEdUtils();
	~FSLEdUtils() = default;

	/* Functionalities */
	// Write the semantic map
	static void WriteSemanticMap(UWorld* World, bool bOverwrite = false);
};
