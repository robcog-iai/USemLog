// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
* Render types
*/
UENUM()
enum class ESLVisRenderType : uint8
{
	Color					UMETA(DisplayName = "Color"),
	Depth					UMETA(DisplayName = "Depth"),
	Mask					UMETA(DisplayName = "Mask"),
	Normal					UMETA(DisplayName = "Normal"),
	Specular				UMETA(DisplayName = "Specular"),
};

// Static helper functions
struct FSLVisHelper
{
	// Get view type suffix
	FORCEINLINE static FString GetRenderTypeSuffix(ESLVisRenderType RenderType);

	// Get render type as command string
	FORCEINLINE static FString GetRenderTypeAsCommandString(ESLVisRenderType RenderType);

	// Get render type as string
	FORCEINLINE static FString GetRenderTypeAsString(ESLVisRenderType RenderType);

	// Get image filename
	FORCEINLINE static FString CreateImageFilename(float Timestamp, const FString& ViewName, ESLVisRenderType RenderType);
};

// Get view type suffix
FString FSLVisHelper::GetRenderTypeSuffix(ESLVisRenderType RenderType)
{
	if (RenderType == ESLVisRenderType::Color)
	{
		return FString("C"); // Color
	}
	else if (RenderType == ESLVisRenderType::Depth)
	{
		return FString("D"); // Depth
	}
	else if (RenderType == ESLVisRenderType::Normal)
	{
		return FString("N"); // Normal
	}
	else if (RenderType == ESLVisRenderType::Mask)
	{
		return FString("M"); // Mask
	}
	else
	{
		// Unsupported buffer type
		return FString("Unknown");
	}
}

// Get render type as command string
FString FSLVisHelper::GetRenderTypeAsCommandString(ESLVisRenderType RenderType)
{
	if (RenderType == ESLVisRenderType::Color)
	{
		return FString(""); // Color
	}
	else if (RenderType == ESLVisRenderType::Depth)
	{
		return FString("SLSceneDepthWorldUnits"); // SceneDepthWorldUnits // SceneDepth
	}
	else if (RenderType == ESLVisRenderType::Normal)
	{
		return FString("WorldNormal"); // Normal
	}
	else if (RenderType == ESLVisRenderType::Mask)
	{
		return FString("SLMask"); // Mask
	}
	else
	{
		// Unsupported buffer type
		return FString("Unknown");
	}
}

// Get render type as string
FString FSLVisHelper::GetRenderTypeAsString(ESLVisRenderType RenderType)
{
	if (RenderType == ESLVisRenderType::Color)
	{
		return FString("Color"); 
	}
	else if (RenderType == ESLVisRenderType::Depth)
	{
		return FString("Depth"); 
	}
	else if (RenderType == ESLVisRenderType::Normal)
	{
		return FString("Normal");
	}
	else if (RenderType == ESLVisRenderType::Mask)
	{
		return FString("Mask");
	}
	else
	{
		// Unsupported buffer type
		return FString("Unknown");
	}
}

// Get image filename
FString FSLVisHelper::CreateImageFilename(float Timestamp, const FString& ViewName, ESLVisRenderType RenderType)
{
	return FString::Printf(TEXT("SLVis_%s_%s_%s.png"),
		*ViewName,
		*FString::SanitizeFloat(Timestamp).Replace(TEXT("."), TEXT("-")),
		*FSLVisHelper::GetRenderTypeSuffix(RenderType));
}

/**
* Keeps track of the vision logging processes duration
*/
struct FSLVisDurationsLogger
{
	// Ctor
	FSLVisDurationsLogger() 
	{
		Reset();
	};

	// Reset all values
	void Reset() 
	{
		ST = 0.0;
		ET = 0.0;
		ScrubReq = 0.0;
		ScrubCb = 0.0;
		ScreenshotReq = 0.0;
		ScreenshotCb = 0.0;
	}

	// Total time
	void SetStartTime() { SetTime(ST); };
	void SetEndTime(bool bLogDuration = false) { SetTime(ET); if (bLogDuration) { LogTotalDuration(); }; };
	double TotalDuration() const { return ET - ST; };
	void LogTotalDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d TotalDuration=%lf;"), *FString(__func__), __LINE__, TotalDuration()); };

	// Scrub req to callback
	void SetScrubRequestTime() { SetTime(ScrubReq); };
	void SetScrubCbTime(bool bLogDuration = false) { SetTime(ScrubCb); if (bLogDuration) { LogScrubDuration(); };};
	double ScrubDuration() const {	return ScrubCb - ScrubReq; };
	void LogScrubDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScrubDuration=%lf;"), *FString(__func__), __LINE__, ScrubDuration()); };

	// Screenshot req to callback
	void SetScreenshotRequestTime() { SetTime(ScreenshotReq); };
	void ScreenshotCbTime(bool bLogDuration = false) { SetTime(ScreenshotCb); if (bLogDuration) { LogScreenshotDuration(); }; };
	double ScreenshotDuration() const {	return ScreenshotCb - ScreenshotReq; };
	void LogScreenshotDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScreenshotDuration=%lf;"), *FString(__func__), __LINE__, ScreenshotDuration()); };

private:
	// Set variable to the time
	FORCEINLINE void SetTime(double &Time) { Time = FPlatformTime::Seconds(); };

private:
	double ST;
	double ET;

	double ScrubReq;
	double ScrubCb;

	double ScreenshotReq;
	double ScreenshotCb;
};

/**
* Keeps track of the vision logging progress
*/
struct FSLVisProgressLogger
{
	// The current replay timestamp
	float CurrTs;

	// Total demo time
	float TotalTime;

	// Number of images to be processed
	uint32 NumImgsToProcess;

	// Number of processed images 
	uint32 NumProcessedImgs;

	// Init values with calculations
	void Init(float InTotalTime, float ScrubRate, uint32 NumViews, uint32 NumRenderTypes)
	{
		CurrTs = 0.f;
		NumProcessedImgs = 0;
		TotalTime = InTotalTime;
		const uint32 NumScrubs = (uint32)(InTotalTime / ScrubRate) + 1;
		const uint32 NumImgsPerScrub = NumViews * NumRenderTypes;
		NumProcessedImgs = NumScrubs * NumImgsPerScrub;
	}


	// Init values
	void Init(float InTotalTime, uint32 InNumImgsToprocess)
	{
		CurrTs = 0.f;
		NumProcessedImgs = 0;
		TotalTime = InTotalTime;
		NumImgsToProcess = InNumImgsToprocess;
	}

	// Get state as string
	FString ToString() const 
	{
		return FString::Printf(TEXT("Time=%.9g/%.9g; Imgs=%d/%d;"), CurrTs, TotalTime, NumImgsToProcess, NumProcessedImgs);
	}

	// Log progress
	void LogProgress() const
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s"), *FString(__func__), __LINE__, *ToString());
	}
};

