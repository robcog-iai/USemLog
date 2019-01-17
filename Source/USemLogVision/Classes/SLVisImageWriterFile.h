// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
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

	// Write data
	virtual void Write(const TArray<uint8>& InCompressedBitmap,
		const FSLVisImageMetadata& Metadata) override;
private:
	// Set the suffix of the file depending on the view type
	FString GetSuffix(const FName& ViewType);

private:
	
	// Path where to save the images
	FString DirPath;

	// Suffix of the filename
	FString FilenameSuffix;
};
