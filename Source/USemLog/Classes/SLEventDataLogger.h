// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OwlEvents.h"
#include "SLEventDataLogger.generated.h"

/**
* Events owl document template types
*/
UENUM(BlueprintType)
enum class EEventsTemplate : uint8
{
	NONE					UMETA(DisplayName = "None"),
	Default					UMETA(DisplayName = "Default"),
	IAI						UMETA(DisplayName = "IAI"),
};

/**
 * Event (symbolic) data logger
 */
UCLASS()
class USEMLOG_API USLEventDataLogger : public UObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLEventDataLogger();

	// Destructor
	~USLEventDataLogger();

	// Init Logger
	void Init(const FString& InLogDirectory, const FString& InEpisodeId, EEventsTemplate TemplateType);

	// Start logger
	void Start();

	// Finish logger
	void Finish();

private:
	// Write events to file
	bool WriteToFile();

	// Directory where to log
	FString LogDirectory;

	// Unique id of the episode
	FString EpisodeId;

	// Type of owl template to write the events to
	EEventsTemplate OwlDocTemplate;

	//// Array of pending events
	////TArray<IEvent> Events;

	//// Owl document of the finished events
	//TSharedPtr<FOwlEvents> EventsDoc;
};