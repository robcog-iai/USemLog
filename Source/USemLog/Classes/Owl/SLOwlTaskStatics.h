// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "EngineMinimal.h"
#include "Owl/SLOwlTask.h"

/**
* Helper functions for generating owl experiment documents
*/
struct USEMLOG_API FSLOwlTaskStatics
{
	/* Events doc (experiment) template creation */
	// Create Default experiment
	static TSharedPtr<FSLOwlTask> CreateDefaultTask(
		const FString& InDocId,
		const FString& InDocPrefix = "log",
		const FString& InDocOntologyName = "ameva_log");

	// Write task to file
	static void WriteToFile(TSharedPtr<FSLOwlTask> Task, const FString& Path, bool bOverwrite);
};
