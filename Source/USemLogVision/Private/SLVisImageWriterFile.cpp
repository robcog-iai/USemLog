// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterFile.h"
#include "FileHelper.h"

// Ctor
USLVisImageWriterFile::USLVisImageWriterFile()
{
	bIsInit = false;
}

// Dtor
USLVisImageWriterFile::~USLVisImageWriterFile()
{
}

// Init
void USLVisImageWriterFile::Init(const FSLVisImageWriterParams& InParams)
{
	// Set path to store the images
	DirPath = InParams.Location + InParams.EpisodeId + TEXT("_RP/");
	FPaths::RemoveDuplicateSlashes(DirPath);
	bIsInit = true;
}

// Finish
void USLVisImageWriterFile::Finish()
{
	if (bIsInit)
	{
		bIsInit = false;
	}
}

// Write the images at the timestamp
void USLVisImageWriterFile::Write(float Timestamp, const TArray<FSLVisImageData>& ImagesData)
{
	// Set path and filename
	FString TsStr = FString::SanitizeFloat(Timestamp).Replace(TEXT("."), TEXT("-"));

	// Iterate the images from the current timestamp
	for (const auto& Img : ImagesData)
	{
		FString Filename = ISLVisImageWriterInterface::GetImageFilename(Timestamp, Img.Metadata.Label, Img.Metadata.ViewType);
		FString ImgPath = DirPath + "/" + Filename;
		FPaths::RemoveDuplicateSlashes(ImgPath);
		// Save to file
		FFileHelper::SaveArrayToFile(Img.Data, *ImgPath);
	}
}
