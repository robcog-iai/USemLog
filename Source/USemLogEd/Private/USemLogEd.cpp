// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "USemLogEd.h"
#include "USemLogEdMode.h"

#define LOCTEXT_NAMESPACE "FUSemLogEdModule"

void FUSemLogEdModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FUSemLogEdMode>(FUSemLogEdMode::EM_USemLogEdModeId, LOCTEXT("USemLogEdModeName", "Semantic Logger"), FSlateIcon(), true);
}

void FUSemLogEdModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FUSemLogEdMode::EM_USemLogEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSemLogEdModule, USemLogEd)