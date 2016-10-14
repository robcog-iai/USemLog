// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "USemLogPrivatePCH.h"

// Define logging types
DEFINE_LOG_CATEGORY(SemLog);
DEFINE_LOG_CATEGORY(SemLogRaw);
DEFINE_LOG_CATEGORY(SemLogEvent);
DEFINE_LOG_CATEGORY(SemLogMap);

#define LOCTEXT_NAMESPACE "FUSemLogModule"

void FUSemLogModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FUSemLogModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSemLogModule, USemLog)