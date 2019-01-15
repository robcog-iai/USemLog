// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisImageWriterMongo.h"

// Ctor
USLVisImageWriterMongo::USLVisImageWriterMongo()
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d !!!!!!!!!!! "), TEXT(__FUNCTION__), __LINE__);
}

// Dtor
USLVisImageWriterMongo::~USLVisImageWriterMongo()
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d !!!!!!!!!!! "), TEXT(__FUNCTION__), __LINE__);
}

// Init
void USLVisImageWriterMongo::Init(const FSLVisImageWriterParams& InParams)
{

}

// Write data
void USLVisImageWriterMongo::Write(const TArray<uint8>& InCompressedBitmap)
{

}