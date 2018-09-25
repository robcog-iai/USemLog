// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "USemLogMap.h"

// Define logging types
DEFINE_LOG_CATEGORY(LogSemLogMap);

#define LOCTEXT_NAMESPACE "FUSemLogMap"

void FUSemLogMap::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUSemLogMap::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSemLogMap, USemLogMap)