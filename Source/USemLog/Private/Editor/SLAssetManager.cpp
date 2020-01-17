// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLAssetManager.h"

// Ctor
USLAssetManager::USLAssetManager()
{
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
}

// Dtor
USLAssetManager::~USLAssetManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish();		
	}
}

// Init scanner
void USLAssetManager::Init(const FString& TaskId, const FString& ServerIp,
	uint16 ServerPort, ESLAssetAction InAction, bool bOverwrite)
{
	if (!bIsInit)
	{
		if (InAction == ESLAssetAction::NONE)
		{
			return;
		}

		Action = InAction;

		if (Action == ESLAssetAction::Move)
		{
			// TODO
			const FString Path = "SemLogAssets/"+ TaskId;
			if (!MoveAssets(Path))
			{
				return;
			}
		}
		else if (Action == ESLAssetAction::MoveAndUpload)
		{
			// TODO
			const FString Path = "SemLogAssets/" + TaskId;
			if (!MoveAssets(Path))
			{
				return;
			}

			// Change to the remaining action
			Action = ESLAssetAction::Upload;
		}

		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			if (!DBHandler.Connect(TaskId, ServerIp, ServerPort, Action, bOverwrite))
			{
				return;
			}
		}

		bIsInit = true;
	}
}

// Start scanning, set camera into the first pose and trigger the screenshot
void USLAssetManager::Start()
{
	if (!bIsStarted && bIsInit)
	{

		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			DBHandler.Execute();
		}		

		bIsStarted = true;
	}
}

// Finish scanning
void USLAssetManager::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if (Action == ESLAssetAction::Download || Action == ESLAssetAction::Upload)
		{
			DBHandler.Disconnect();
		}
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Move assets to path
bool USLAssetManager::MoveAssets(const FString & Path)
{
	return false;
}
