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
	float SkipNewEntryTolerance;

	// Constructor
	FSLVisImageWriterParams(
		const FString& InLocation,
		const FString& InEpisodeId,
		float InSkipNewEntryTolerance = 0.f,
		const FString& InServerIp = "",
		uint16 InServerPort = 0) :
		Location(InLocation),
		EpisodeId(InEpisodeId),
		SkipNewEntryTolerance(InSkipNewEntryTolerance),
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

	// Timestamp when the image was rendered in the replay
	float ReplayTimestamp;

	// Constructor
	FSLVisImageMetadata(const FName& InViewType,
		const FString& InCameraLabel,
		int32 InResX,
		int32 InResY,
		float InReplayTimestamp) :
		ViewType(InViewType),
		Label(InCameraLabel),
		ResX(InResX),
		ResY(InResY),
		ReplayTimestamp(InReplayTimestamp)
	{};
};

/**
* Images data with metadata
*/
struct FSLVisImageData
{	
	// Metadata
	FSLVisImageMetadata Metadata;

	// Binary data
	TArray<uint8> BinaryData;

	// Ctor
	FSLVisImageData(const FSLVisImageMetadata& InMetadata, const TArray<uint8>& InBinaryData) :
		Metadata(InMetadata), BinaryData(InBinaryData)
	{};
};

///////////////////////////////////////////////////////////////////////

/**
* Images data 
*/
struct FSLVisImageData2
{
	// Render type
	FString RenderType;

	// Binary data
	TArray<uint8> BinaryData;
};


/**
* Semantic entities data from the view
*/
struct FSLVisSemanticEntities
{
	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString  Class;

	// Number of pixels belonging to the entity from the image
	int32 NumPixels;

	// Indexes of the color
	TArray<int32> Indexes;
};


/**
* Data data from the view
*/
struct FSLVisViewData
{
	// Name of the current view
	FString ViewName;

	// Image resolution
	FVector2D Resolution;

	// Data about the entities visible in the view
	TArray<FSLVisSemanticEntities> SemanticEntities;

	// Image data of the given render type
	TArray<FSLVisImageData2> ImagesData;
};


/**
* Collection of all the views data in the current rendered timestamp
*/
struct FSLVisDataStamped
{
	// Array of the camera views data
	TMap<FString, FSLVisViewData> ViewsData;

	// The timestamp when the images are rendered
	float Timestamp;
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
	FORCEINLINE static FString CreateImageFilename(float Timestamp, const FString& Label, const FName& ViewType);

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
	else if (ViewType.IsEqual("SceneDepth") || ViewType.IsEqual("SLSceneDepth") || ViewType.IsEqual("SLSceneDepthWorldUnits"))
	{
		return FString("D"); // Depth
	}
	else if (ViewType.IsEqual("WorldNormal"))
	{
		return FString("N"); // Normal
	}
	else if (ViewType.IsEqual("Mask") || ViewType.IsEqual("SLMask"))
	{
		return FString("M"); // Normal
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
FString ISLVisImageWriterInterface::CreateImageFilename(float Timestamp, const FString& Label, const FName& ViewType)
{
	return FString::Printf(TEXT("SLVis_%s_%s_%s.png"),
		*Label,
		*FString::SanitizeFloat(Timestamp).Replace(TEXT("."), TEXT("-")),
		*ISLVisImageWriterInterface::GetViewTypeSuffix(ViewType));
}