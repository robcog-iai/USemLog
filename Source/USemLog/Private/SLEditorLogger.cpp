// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEditorLogger.h"
#include "Editor/SLEditorToolkit.h"

// Constructor
USLEditorLogger::USLEditorLogger()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Destructor
USLEditorLogger::~USLEditorLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init Logger
void USLEditorLogger::Init(const FString& InTaskId)
{
	if (!bIsInit)
	{
		TaskId = InTaskId;
		bIsInit = true;
	}
}

// Start logger
void USLEditorLogger::Start(
	bool bWriteSemanticMap,
	bool bClearTags,
	const FString& ClearTagType,
	const FString& ClearKeyType,
	bool bOverwriteProperties,
	bool bWriteClassProperties,
	bool bWriteUniqueIdProperties,
	bool bWriteVisualMaskProperties,
	int32 VisualMaskColorMinDistance,
	bool bRandomMaskGenerator,
	bool bTagStaticEntities)
{
	if (!bIsStarted && bIsInit)
	{
		if(bWriteSemanticMap)
		{
			FSLEditorToolkit::WriteSemanticMap(GetWorld(), TaskId);
		}

		if(bClearTags)
		{
			FSLEditorToolkit::ClearTags(GetWorld(), ClearTagType, ClearKeyType);
		}

		if(bWriteClassProperties)
		{
			FSLEditorToolkit::WriteClassProperties(GetWorld(), bOverwriteProperties);
		}

		if(bWriteUniqueIdProperties)
		{
			FSLEditorToolkit::WriteUniqueIdProperties(GetWorld(), bOverwriteProperties);
		}

		if(bWriteVisualMaskProperties)
		{
			FSLEditorToolkit::WriteUniqueMaskProperties(GetWorld(), bOverwriteProperties, VisualMaskColorMinDistance, bRandomMaskGenerator);
		}

		if(bTagStaticEntities)
		{
			FSLEditorToolkit::TagNonMovableEntities(GetWorld(), bOverwriteProperties);
		}
		
		bIsStarted = true;
	}
}

// Finish logger
void USLEditorLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if(!bForced)
		{
			// Give warnings for the users to fix any duplicate camera view class names;
			FSLEditorToolkit::CheckForVisionCameraClassNameDuplicates(GetWorld());
			QuitEditor();
		}
	}
}

// Safely quit the editor
void USLEditorLogger::QuitEditor()
{
	//FGenericPlatformMisc::RequestExit(false);
	//
	//FGameDelegates::Get().GetExitCommandDelegate().Broadcast();
	//FPlatformMisc::RequestExit(0);
#if WITH_EDITOR
	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
#endif // WITH_EDITOR
}