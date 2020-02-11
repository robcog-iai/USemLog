// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SLDataVisQueries.generated.h"

/**
 * Query structure
 */
USTRUCT()
struct FSLVisQuery
{
	GENERATED_BODY()

	// Task id (mongo database name)
	UPROPERTY(EditAnywhere)
	FString TaskId;

	// Episode id (mongo collection name)
	UPROPERTY(EditAnywhere)
	FString EpisodeId;
};

/**
 * 
 */
UCLASS(ClassGroup = (SL), meta = (DisplayName = "SL Data Vis Queries"))
class USLDataVisQueries : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ServerIP = "localhost";

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint16 ServerPort = 27017;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	TArray<FSLVisQuery> Queries;
};
