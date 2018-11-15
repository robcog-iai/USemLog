// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "USemLogVision.h"

// Define logging types
DEFINE_LOG_CATEGORY(LogSLVis);

#define LOCTEXT_NAMESPACE "FUSemLogVision"

void FUSemLogVision::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUSemLogVision::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSemLogVision, USemLogVision)