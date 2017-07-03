// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SLOwl.h"
#include "SLEventData.generated.h"

/**
 * 
 */
UCLASS()
class SEMLOG_API USLEventData : public UObject
{
	GENERATED_BODY()

public:
	// Default constructor
	USLEventData();

	// Destructor
	~USLEventData();

	// Initialise logger
	bool Init(const FString InEpisodeId, const FString InLogDirectoryPath);

	// Write document to file
	UFUNCTION(BlueprintCallable)
	bool WriteToFile();

	// Event logs document as owl representation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SL)
	FOwlDocument OwlDocument;

	// Check if the logger is initialised
	bool IsInit() const { return bIsInit; };

private:
	// Set document default values
	void SetDefaultValues();

	// Remove document default values
	void RemoveDefaultValues();

	// Insert event to the document
	bool InsertEvent(const TPair<AActor*, TMap<FString, FString>>& ActorWithProperties);

	// Logging directory path
	FString LogDirectoryPath;

	// Id of the log file
	FString EpisodeId;

	// Logger initialised
	bool bIsInit;

	// Shows if the default values of the owl document have been set
	bool bOwlDefaultValuesSet;
};
