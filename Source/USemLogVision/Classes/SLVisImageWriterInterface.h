// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SLVisImageWriterInterface.generated.h"

/**
* Parameters for creating an image data writer
*/
struct FSLVisImageWriterParams
{
	// Location where to save the data (filename/database name etc.)
	FString Location;

	// Episode unique id
	FString EpisodeId;

	// Server ip (optional)
	FString ServerIp;

	// Server Port (optional)
	uint16 ServerPort;

	// Constructor
	FSLVisImageWriterParams(
		const FString& InLocation,
		const FString& InEpisodeId,
		const FString& InServerIp = "",
		uint16 InServerPort = 0) :
		Location(InLocation),
		EpisodeId(InEpisodeId),
		ServerIp(InServerIp),
		ServerPort(InServerPort)
	{};
};

/**
* Image metadata
*/
struct FSLVisImageMetadata
{
	// Timestamp
	float Timestamp;

	// View type
	FName ViewType;

	// Camera name
	FString CameraLabel;

	// Image resolution X
	int32 ResX;

	// Image resolution Y
	int32 ResY;

	// Constructor
	FSLVisImageMetadata(float InTimestamp,
		const FName& InViewType,
		const FString& InCameraLabel,
		int32 InResX,
		int32 InResY) :
		Timestamp(InTimestamp),
		ViewType(InViewType),
		CameraLabel(InCameraLabel),
		ResX(InResX),
		ResY(InResY)
	{};
};



/**
* Dummy class needed to support Cast<ISLVisImageWriterInterface>(Object).
*/
UINTERFACE(Blueprintable)
class USLVisImageWriterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Base class for image data writer
 */
class ISLVisImageWriterInterface
{
	GENERATED_BODY()

public:
	// Init the writer
	virtual void Init(const FSLVisImageWriterParams& InParams) = 0;

	// Write the image
	virtual void Write(const TArray<uint8>& InCompressedBitmap,
		const FSLVisImageMetadata& Metadata) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

protected:
	// Flag to show if it is valid
	bool bIsInit;
};
