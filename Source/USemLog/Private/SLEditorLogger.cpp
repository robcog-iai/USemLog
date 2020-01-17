// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEditorLogger.h"
#include "Editor/SLEditorToolkit.h"
#include "Editor/SLMaskCalibrationTool.h"
#include "Editor/SLAssetManager.h"

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
void USLEditorLogger::Init(const FString& InTaskId, const FString& ServerIp,
	uint16 ServerPort, ESLAssetAction InAction, bool bCalibrateRenderedMaskColors, bool bMaskColorsOnlyDemo, bool bOverwrite)
{
	if (!bIsInit)
	{
		TaskId = InTaskId;

		if (InAction == ESLAssetAction::NONE)
		{
			if (bCalibrateRenderedMaskColors)
			{
				CalibrationTool = NewObject<USLMaskCalibrationTool>(this);
				CalibrationTool->Init(bMaskColorsOnlyDemo);
			}
		}
		else
		{
			AssetManager = NewObject<USLAssetManager>(this);
			AssetManager->Init(InTaskId, ServerIp, ServerPort, InAction, bOverwrite);
		}

		bIsInit = true;
	}
}

// Start logger
void USLEditorLogger::Start(const FSLEditorLoggerParams& InParams)
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;

		if (AssetManager)
		{
			AssetManager->Start();	
		}
		else
		{
			if (InParams.bWriteSemanticMap)
			{
				FSLEditorToolkit::WriteSemanticMap(GetWorld(), TaskId);
			}

			if (InParams.bClearAllTags)
			{
				FSLEditorToolkit::ClearTags(GetWorld(), InParams.TagTypeToClear, InParams.TagKeyToClear);
			}

			if (InParams.bWriteClassTags)
			{
				FSLEditorToolkit::WriteClassProperties(GetWorld(), InParams.bOverwrite);
			}

			if (InParams.bWriteUniqueIdTags)
			{
				FSLEditorToolkit::WriteUniqueIdProperties(GetWorld(), InParams.bOverwrite);
			}

			if (InParams.bWriteUniqueMaskColors)
			{
				FSLEditorToolkit::WriteUniqueMaskProperties(GetWorld(), InParams.bOverwrite, InParams.MinColorManhattanDistance, InParams.bUseRandomColorGeneration);
			}

			//if (InParams.bMarkStaticEntities)
			//{
			//	FSLEditorToolkit::TagNonMovableEntities(GetWorld(), bOverwriteProperties);
			//}

			if (CalibrationTool)
			{
				CalibrationTool->Start();
			}
		}
	}
}

// Finish logger
void USLEditorLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if(!bForced)
		{
			if(CalibrationTool)
			{
				CalibrationTool->Finish();
			}

			if (AssetManager)
			{
				AssetManager->Finish();
			}

			// Give warnings for the users to fix any duplicate camera view class names;
			FSLEditorToolkit::CheckForVisionCameraClassNameDuplicates(GetWorld());
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
#if WITH_EDITOR
	// Make sure you can quit even if Init or Start could not work out
	if (GEngine)
	{
		GEngine->DeferredCommands.Add(TEXT("QUIT_EDITOR"));
	}
#endif // WITH_EDITOR
}