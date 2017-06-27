// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLRawData.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLRawData : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLRawData();

	// Destructor
	~USLRawData();

	// Log raw data
	UPROPERTY(EditAnywhere, Category = "SL")
	FString Test;
};
