// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SemLogEdModule.h"
#include "SemLogEdMode.h"
#include "SemLogEdStyle.h"

// Define logging types
DEFINE_LOG_CATEGORY(SemLogEd);

#define LOCTEXT_NAMESPACE "FSemLogEdModule"

void FSemLogEdModule::StartupModule()
{
	// Register slate style overrides
	FSemLogEdStyle::Initialize();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FSemLogEdMode>(
		FSemLogEdMode::EM_SemLogEdModeId, 
		LOCTEXT("SemLogEdModeName", "Semantic Logger"), 
		FSlateIcon(FSemLogEdStyle::Get()->GetStyleSetName(), "LevelEditor.SemLogEd", "LevelEditor.SemLogEd.Small"),
		true);
}

void FSemLogEdModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FSemLogEdMode::EM_SemLogEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSemLogEdModule, SemLogEd)