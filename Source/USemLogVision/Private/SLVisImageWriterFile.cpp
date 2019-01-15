// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterFile.h"

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

}

// Write data
void USLVisImageWriterFile::Write(const TArray<uint8>& InCompressedBitmap,
	float Timestamp, FName ViewType, int32 TargetIndex)
{

}