// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "USemLogEd.h"
#include "SLEdMode.h"
#include "SLEdStyle.h"

// Define logging types
DEFINE_LOG_CATEGORY(LogSLEd);

#define LOCTEXT_NAMESPACE "FUSemLogEd"

void FUSemLogEd::StartupModule()
{
	// Register slate style overrides
	FSLEdStyle::Initialize();
	FSLEdStyle::ReloadTextures();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FSLEdMode>(
		FSLEdMode::EM_SLEdModeId, 
		LOCTEXT("SemLogEdModeName", "Semantic Logger"), 
		FSlateIcon(FSLEdStyle::Get().GetStyleSetName(), "LevelEditor.SemLogEd", "LevelEditor.SemLogEd.Small"),
		true);
}

void FUSemLogEd::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FSLEdMode::EM_SLEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUSemLogEd, USemLogEd)