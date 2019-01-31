// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
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

	// Minimum time offset for a new entry
	float NewEntryTimeRange;

	// Constructor
	FSLVisImageWriterParams(
		const FString& InLocation,
		const FString& InEpisodeId,
		float InNewEntryTimeRange = 0.f,
		const FString& InServerIp = "",
		uint16 InServerPort = 0) :
		Location(InLocation),
		EpisodeId(InEpisodeId),
		NewEntryTimeRange(InNewEntryTimeRange),
		ServerIp(InServerIp),
		ServerPort(InServerPort)
	{};
};

/**
* Image metadata
*/
struct FSLVisImageMetadata
{
	// View type
	FName ViewType;

	// Camera name
	FString Label;

	// Image resolution X
	int32 ResX;

	// Image resolution Y
	int32 ResY;

	// Constructor
	FSLVisImageMetadata(const FName& InViewType,
		const FString& InCameraLabel,
		int32 InResX,
		int32 InResY) :
		ViewType(InViewType),
		Label(InCameraLabel),
		ResX(InResX),
		ResY(InResY)
	{};
};

/**
* Images data with metadata
*/
struct FSLVisImageData
{	
	// Metadata
	FSLVisImageMetadata Metadata;

	// Data
	TArray<uint8> Data;

	// Ctor
	FSLVisImageData(const FSLVisImageMetadata& InMetadata, const TArray<uint8>& InData) :
		Metadata(InMetadata), Data(InData)
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

	// Called when done writing
	virtual void Finish() = 0;

	// Write the images at the timestamp
	virtual void Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

	// Get view type suffix
	FORCEINLINE static FString GetViewTypeSuffix(const FName& ViewType);

	// Get view type name
	FORCEINLINE static FString GetViewTypeName(const FName& ViewType);

	// Get image filename
	FORCEINLINE static FString GetImageFilename(float Timestamp, const FString& Label, const FName& ViewType);

protected:
	// Flag to show if it is valid
	bool bIsInit;
};

// Get view type suffix
FString ISLVisImageWriterInterface::GetViewTypeSuffix(const FName& ViewType)
{
	if (ViewType.IsEqual(NAME_None))
	{
		return FString("C"); // Color
	}
	else if (ViewType.IsEqual("SceneDepth"))
	{
		return FString("D"); // Depth
	}
	else if (ViewType.IsEqual("WorldNormal"))
	{
		return FString("N"); // Normal
	}
	else
	{
		// Unsupported buffer type
		return FString("Unknown");
	}
}

// Get view type name
FString ISLVisImageWriterInterface::GetViewTypeName(const FName& ViewType)
{
	if (ViewType.IsEqual(NAME_None))
	{
		return FString("Color"); // Color
	}
	else
	{
		return ViewType.ToString();
	}
}

// Get image filename
FString ISLVisImageWriterInterface::GetImageFilename(float Timestamp, const FString& Label, const FName& ViewType)
{
	return FString::Printf(TEXT("SLVis_%s_%s_%s.png"),
		*Label,
		*FString::SanitizeFloat(Timestamp).Replace(TEXT("."), TEXT("-")),
		*ISLVisImageWriterInterface::GetViewTypeSuffix(ViewType));
}