// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterFile.h"
#include "FileHelper.h"

// Ctor
USLVisImageWriterFile::USLVisImageWriterFile()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d !!!!!!!!!!! "), TEXT(__FUNCTION__), __LINE__);
}

// Dtor
USLVisImageWriterFile::~USLVisImageWriterFile()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d !!!!!!!!!!! "), TEXT(__FUNCTION__), __LINE__);
}

// Init
void USLVisImageWriterFile::Init(const FSLVisImageWriterParams& InParams)
{
	// Set path to store the images
	DirPath = InParams.Location + InParams.EpisodeId + TEXT("/");
	FPaths::RemoveDuplicateSlashes(DirPath);
}

// Write data
void USLVisImageWriterFile::Write(const TArray<uint8>& InCompressedBitmap,
	const FSLVisImageMetadata& Metadata)
{
	// Set path and filename
	FString TsStr = FString::SanitizeFloat(Metadata.Timestamp).Replace(TEXT("."), TEXT("-"));
	FString Filename = FString::Printf(TEXT("SLVis_%s_%s_%s.png"),
		*Metadata.CameraLabel,
		*TsStr,
		*USLVisImageWriterFile::GetSuffix(Metadata.ViewType));
	FString ImgPath = DirPath + "/" + Filename;
	FPaths::RemoveDuplicateSlashes(ImgPath);

	UE_LOG(LogTemp, Error, TEXT("%s::%d !!!!!!!!!!! Path=%s"),
		TEXT(__FUNCTION__), __LINE__, *ImgPath);

	// Save to file
	FFileHelper::SaveArrayToFile(InCompressedBitmap, *ImgPath);
}

// Set the suffix of the file depending on the view type
FString USLVisImageWriterFile::GetSuffix(const FName& ViewType)
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