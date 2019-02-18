// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

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
	void LogTotalDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d TotalDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, TotalDuration()); };

	// Scrub req to callback
	void SetScrubRequestTime() { SetTime(ScrubReq); };
	void SetScrubCbTime(bool bLogDuration = false) { SetTime(ScrubCb); if (bLogDuration) { LogScrubDuration(); };};
	double ScrubDuration() const {	return ScrubCb - ScrubReq; };
	void LogScrubDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScrubDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, ScrubDuration()); };

	// Screenshot req to callback
	void SetScreenshotRequestTime() { SetTime(ScreenshotReq); };
	void ScreenshotCbTime(bool bLogDuration = false) { SetTime(ScreenshotCb); if (bLogDuration) { LogScreenshotDuration(); }; };
	double ScreenshotDuration() const {	return ScreenshotCb - ScreenshotReq; };
	void LogScreenshotDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScreenshotDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, ScreenshotDuration()); };

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
	float DemotTime;
};

