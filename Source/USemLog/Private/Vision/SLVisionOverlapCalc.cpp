// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionOverlapCalc.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Async.h"
#include "FileHelper.h"

// Constructor
USLVisionOverlapCalc::USLVisionOverlapCalc() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
}

// Destructor
USLVisionOverlapCalc::~USLVisionOverlapCalc()
{
}

// 
void USLVisionOverlapCalc::Init()
{
	if (!bIsInit)
	{
		bIsInit = true;
	}
}

// 
void USLVisionOverlapCalc::Start()
{
	if (!bIsStarted && bIsInit)
	{
		bIsStarted = true;
	}
}

// 
void USLVisionOverlapCalc::Finish()
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Trigger the screenshot on the game thread
void USLVisionOverlapCalc::RequestScreenshot()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		GetHighResScreenshotConfig().FilenameOverride = "OverlapTest";
		//GetHighResScreenshotConfig().SetForce128BitRendering(true);
		//GetHighResScreenshotConfig().SetHDRCapture(true);
		//ViewportClient->Viewport->TakeHighResScreenShot();
	});
}

// Called when the screenshot is captured
void USLVisionOverlapCalc::ScreenshotCB(int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
{
}