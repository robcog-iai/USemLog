// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
* View modes
*/
UENUM()
enum class ESLMetaScannerViewMode : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Color					UMETA(DisplayName = "Color"),
	Unlit					UMETA(DisplayName = "Unlit"),
	White					UMETA(DisplayName = "Mask"),
	Depth					UMETA(DisplayName = "Depth"),
	Normal					UMETA(DisplayName = "Normal"),
};


/**
* Parameters for creating scanning the semantic map items
*/
struct FSLMetaScannerParams
{
	// Scan image resolution
	FIntPoint Resolution;

	// Number of camera poses on the sphere pointed toward the object
	int32 NumberOfScanPoints;

	// The maximum volume (cm^3) of an item that should be scanned (0 = no limit)
	float MaxItemVolume;

	// The distance of the camera to the scanned item (0 = calculated relative to the object size)
	float CameraDistanceToScanItem;

	// Scan view modes
	TArray<ESLMetaScannerViewMode> ViewModes;

	// Save the scanned images locally
	bool bIncludeScansLocally;

	// Default constructor
	FSLMetaScannerParams() {};

	// Constructor
	FSLMetaScannerParams(
		FIntPoint InScanResolution,
		int32 InNumberOfScanPoints,
		float InMaxScanItemVolume,
		float InCameraDistanceToScanItem,
		bool bNewIncludeScansLocally = false)
		:
		Resolution(InScanResolution),
		NumberOfScanPoints(InNumberOfScanPoints),
		MaxItemVolume(InMaxScanItemVolume),
		CameraDistanceToScanItem(InCameraDistanceToScanItem),
		bIncludeScansLocally(bNewIncludeScansLocally)
	{};
};

/**
 * One camera position scan data (number of pixels, rendered images array, object image bounds)
 */
struct FSLScanPoseData
{
	// Number of pixels occupied by the item in the image
	int32 NumPixels;

	// The min bounds coordinates of the image
	FIntPoint MinBB;

	// The max bounds coordinates of the image
	FIntPoint MaxBB;

	// Camera pose
	FTransform CameraPose;

	// Array of image data pair, render type name to binary data
	TArray<TPair<FString, TArray<uint8>>> Images;
};
