// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Vision/SLVisionOverlapCalc.h"

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