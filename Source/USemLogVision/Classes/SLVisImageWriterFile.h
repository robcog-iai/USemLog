// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisImageWriterInterface.h"
#include "SLVisImageWriterFile.generated.h"

/**
 * 
 */
UCLASS()
class USLVisImageWriterFile : public UObject, public ISLVisImageWriterInterface
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLVisImageWriterFile();

	// Dtor
	~USLVisImageWriterFile();

	// Init
	virtual void Init(const FSLVisImageWriterParams& InParams) override;

	// Called when done writing
	virtual void Finish() override;

	// Write the images at the timestamp
	virtual void Write(const FSLVisStampedData& StampedData) override;

private:	
	// Path where to save the images
	FString DirPath;

	// Suffix of the filename
	FString FilenameSuffix;
};
