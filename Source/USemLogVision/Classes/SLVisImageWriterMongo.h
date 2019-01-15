// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLVisImageWriterInterface.h"
#include "SLVisImageWriterMongo.generated.h"

/**
 * 
 */
UCLASS()
class USLVisImageWriterMongo : public UObject, public ISLVisImageWriterInterface
{
	GENERATED_BODY()
	
public:
	// Ctor
	USLVisImageWriterMongo();

	// Dtor
	~USLVisImageWriterMongo();

	// Init
	virtual void Init(const FSLVisImageWriterParams& InParams) override;

	// Write data
	virtual void Write(const TArray<uint8>& InCompressedBitmap) override;
};
