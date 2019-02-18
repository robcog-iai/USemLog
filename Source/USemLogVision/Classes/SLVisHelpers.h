// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
* Stores and calculates various image processing durations
*/
struct FSLVisDurationLogger
{
	// Ctor
	FSLVisDurationLogger() 
	{
		Reset();
	};

	// Reset all values
	void Reset() 
	{
		ST = 0.0;
		ET = 0.0;
		ScrubRequest = 0.0;
		ScrubCallback = 0.0;
		ScreenshotRequest = 0.0;
		ScreenshotCallback = 0.0;
	}

	// Total time
	void SetStartTime() { SetTime(ST); };
	void SetEndTime(bool bLogDuration = false) { SetTime(ET); if (bLogDuration) { LogTotalDuration(); }; };
	double TotalDuration() const { return ET - ST; };
	void LogTotalDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d TotalDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, TotalDuration()); };

	// Scrub req to callback
	void ScrubReq() { SetTime(ScrubRequest); };
	void ScrubCb(bool bLogDuration = false) { SetTime(ScrubCallback); if (bLogDuration) { LogScrubDuration(); };};
	double ScrubDuration() const {	return ScrubRequest - ScrubCallback; };
	void LogScrubDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScrubDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, ScrubDuration()); };

	// Screenshot req to callback
	void ScreenshotReq() { SetTime(ScreenshotRequest); };
	void ScreenshotCb(bool bLogDuration = false) { SetTime(ScreenshotCallback); if (bLogDuration) { LogScreenshotDuration(); }; };
	double ScreenshotDuration() const {	return ScreenshotRequest - ScreenshotCallback; };
	void LogScreenshotDuration() { UE_LOG(LogTemp, Warning, TEXT("%s::%d ScreenshotDuration=%lf;"), TEXT(__FUNCTION__), __LINE__, ScreenshotDuration()); };



private:
	// Set variable to the time
	FORCEINLINE void SetTime(double &Time) { Time = FPlatformTime::Seconds(); };

private:
	// Whole process time
	double ST;
	double ET;

	// Request and callback timestamps
	double ScrubRequest;
	double ScrubCallback;

	double ScreenshotRequest;
	double ScreenshotCallback;


	//// View start time
	//double ViewStart;

	//// View end time
	//double ViewEnd;

	//// Screenshot request

};

/**
* Stores and calculates various image processing durations
*/
struct FSLVisProgressLogger
{
	float DemotTime;
};

