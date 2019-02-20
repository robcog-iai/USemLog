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
* Images metadatadata
*/
struct FSLVisImageMetaData
{
	// Render type
	FString RenderType;

	// Image type
	FString ImageType; //png, depth resolution?
};



/**
* Images data 
*/
struct FSLVisImageData
{
	// Default ctor
	FSLVisImageData() {};

	// Init ctor
	FSLVisImageData(const FString& InRenderType, const TArray<uint8>& InBinaryData) : RenderType(InRenderType), BinaryData(InBinaryData) {};

	// TODO use FSLVisImageMetaData when more data is available
	// Render type
	FString RenderType;

	// Binary data
	TArray<uint8> BinaryData;
};


/**
* Semantic entities data from the view
*/
struct FSLVisEntitiyData
{
	// Default ctor
	FSLVisEntitiyData() : NumPixels(0) {};

	// Color
	FColor Color;

	// Color in hex
	FString ColorHex;

	// Unique id of the entity
	FString Id;

	// Class of the entity
	FString Class;

	// Number of pixels belonging to the entity from the image
	int32 NumPixels;

	// Indexes where the color is located in the array
	//TArray<int32> Indexes;

	FString ToString() const
	{
		return FString::Printf(TEXT("Color=%s; ColorHex=%s; Id=%s; Class=%s; NumPixels=%d;"),
			*Color.ToString(), *ColorHex, *Id, *Class, NumPixels);
	}
};


/**
* Data data from the view
*/
struct FSLVisViewData
{
	// Default ctor
	FSLVisViewData() {};

	// Name of the current view
	FString ViewName;

	// Image resolution
	FIntPoint Resolution;

	// Data about the entities visible in the view
	TArray<FSLVisEntitiyData> SemanticEntities;

	// Image data of the given render type
	TArray<FSLVisImageData> ImagesData;

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Init view data
	void Init(const FString& InViewName, const FIntPoint& InResolution)
	{
		ViewName = InViewName;
		Resolution = InResolution;
		bIsInit = true;
	}

	// Clear all data
	void Reset()
	{
		ViewName = FString();
		Resolution = FIntPoint(ForceInitToZero);
		SemanticEntities.Empty();
		ImagesData.Empty();
		bIsInit = false;
	}

private:
	// Init state flag
	bool bIsInit;
};


/**
* Collection of all the views data in the current rendered timestamp
*/
struct FSLVisStampedData
{
	// The timestamp when the images are rendered
	float Timestamp;

	// Array of the camera views data
	TArray<FSLVisViewData> ViewsData;

	// Reset the data
	void Reset()
	{
		Timestamp = 0.f;
		ViewsData.Empty();
	}

	// Debug string
	FString ToString() const
	{
		return FString::Printf(TEXT("Timestamp=%f; ViewsData.Num()=%d"), Timestamp, ViewsData.Num());
	}
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
	virtual void Write(const FSLVisStampedData& StampedData) = 0;

	// True if the writer is valid
	bool IsInit() const { return bIsInit; }

	// Get view type suffix
	FORCEINLINE static FString GetRenderTypeSuffix(const FString& RenderType);

	// Get image filename
	FORCEINLINE static FString CreateImageFilename(float Timestamp, const FString& ViewName, const FString& RenderType);

protected:
	// Flag to show if it is valid
	bool bIsInit;
};

// Get view type suffix
FString ISLVisImageWriterInterface::GetRenderTypeSuffix(const FString& RenderType)
{
	if (RenderType.Equals("Color"))
	{
		return FString("C"); // Color
	}
	else if (RenderType.Equals("SceneDepth") || RenderType.Equals("SLSceneDepth") || RenderType.Equals("SLSceneDepthWorldUnits"))
	{
		return FString("D"); // Depth
	}
	else if (RenderType.Equals("WorldNormal"))
	{
		return FString("N"); // Normal
	}
	else if (RenderType.Equals("Mask") || RenderType.Equals("SLMask"))
	{
		return FString("M"); // Normal
	}
	else
	{
		// Unsupported buffer type
		return FString("Unknown");
	}
}

// Get image filename
FString ISLVisImageWriterInterface::CreateImageFilename(float Timestamp, const FString& ViewName, const FString& RenderType)
{
	return FString::Printf(TEXT("SLVis_%s_%s_%s.png"),
		*ViewName,
		*FString::SanitizeFloat(Timestamp).Replace(TEXT("."), TEXT("-")),
		*ISLVisImageWriterInterface::GetRenderTypeSuffix(RenderType));
}