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
	// TODO target index use a class/description/id?
	virtual void Write(const TArray<uint8>& InCompressedBitmap,
		float Timestamp, FName ViewType, int32 TargetIndex) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

protected:
	// Flag to show if it is valid
	bool bIsInit;
};
