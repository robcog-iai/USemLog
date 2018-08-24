// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OwlEvents.h"
#include "EventData/SLContactEvent.h"
#include "SLEventDataLogger.generated.h"

// Forward declaration
class ISLEvent;

/**
* Events owl document template types
*/
UENUM(BlueprintType)
enum class ESLEventsTemplate : uint8
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

protected:
	// 
	virtual void BeginDestroy() override;

	//
	virtual void FinishDestroy() override;

public:
	// Init Logger
	void Init(const FString& InLogDirectory, const FString& InEpisodeId, ESLEventsTemplate TemplateType);

	// Start logger
	void Start();

	// Finish logger
	void Finish();

private:
	// Register for semantic contact events
	void ListenToSemanticContactEvents();

	// Called when a semantic contact is finished
	void OnSemanticContactEvent(TSharedPtr<FSLContactEvent> Event);

	// Write events to file
	bool WriteToFile();

	// Create events doc template
	TSharedPtr<FOwlEvents> CreateEventsDocTemplate(
		ESLEventsTemplate TemplateType, const FString& InDocId);

	// Finish the pending events at the current time
	void FinishPendingEvents(const float EndTime);

	// Directory where to log
	FString LogDirectory;

	// Unique id of the episode
	FString EpisodeId;

	// Type of owl template to write the events to
	ESLEventsTemplate OwlDocTemplate;

	// Array of pending events
	TArray<TSharedPtr<ISLEvent>> PendingEvents;

	// Array of pending events
	TArray<TSharedPtr<ISLEvent>> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FOwlEvents> EventsDoc;
};