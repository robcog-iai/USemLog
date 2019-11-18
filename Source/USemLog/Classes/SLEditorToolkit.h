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
	static void WriteSemanticMap(UWorld* World, bool bOverwrite, const FString& TaskId,
		const FString& Filename = "SemanticMap",
		ESLOwlSemanticMapTemplate Template = ESLOwlSemanticMapTemplate::IAIKitchen);

	// Write class properties
	static void WriteClassProperties(UWorld* World, bool bOverwrite);

	// Write unique id properties
	static void WriteUniqueIdProperties(UWorld* World, bool bOverwrite);
};
