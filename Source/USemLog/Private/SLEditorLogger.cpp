// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEditorLogger.h"
#include "SLEditorToolkit.h"

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
	UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);
}

// Init Logger
void USLEditorLogger::Init(const FString& InTaskId)
{
	if (!bIsInit)
	{
		TaskId = InTaskId;
		UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);
		bIsInit = true;
	}
}

// Start logger
void USLEditorLogger::Start(bool bOverwrite,
	bool bWriteSemanticMap,
	bool bWriteClassProperties,
	bool bWriteUniqueIdProperties,
	bool bWriteVisualMaskProperties,
	bool bGenerateVisualMasksRandomly)
{
	if (!bIsStarted && bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d"), *FString(__func__), __LINE__);

		if(bWriteSemanticMap)
		{
			FSLEditorToolkit::WriteSemanticMap(GetWorld(), bOverwrite, TaskId);
		}

		if(bWriteClassProperties)
		{
			FSLEditorToolkit::WriteClassProperties(GetWorld(), bOverwrite);
		}

		if(bWriteUniqueIdProperties)
		{
			FSLEditorToolkit::WriteUniqueIdProperties(GetWorld(), bOverwrite);
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
			//QuitEditor();
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

	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
}