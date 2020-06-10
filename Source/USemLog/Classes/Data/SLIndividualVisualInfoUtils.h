// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class USLIndividualComponent;

/**
 * Static helpers functions the semantic individual annotation
 */
class USEMLOG_API FSLIndividualVisualInfoUtils
{
public:
	// Add new visual info component to actor (return true if component has been created or modified)
	static bool AddNewVisualInfoComponent(AActor* Actor, bool bOverwrite);
};
