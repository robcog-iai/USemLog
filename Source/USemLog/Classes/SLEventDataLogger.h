// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OwlEvents.h"
#include "EventData/IEvent.h"
#include "SLContactPoolSingleton.h"
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

protected:
	// 
	virtual void BeginDestroy() override;

	//
	virtual void FinishDestroy() override;

public:
	// Init Logger
	void Init(const FString& InLogDirectory, const FString& InEpisodeId, EEventsTemplate TemplateType);

	// Start logger
	void Start();

	// Finish logger
	void Finish();

private:
	// Register for semantic contact events
	void ListenToSemanticContactEvents();

	// Called when a semantic contact is finished
	void OnSemanticContactEvent(IEvent* Event);

	// Write events to file
	bool WriteToFile();

	// Create events doc template
	TSharedPtr<FOwlEvents> CreateEventsDocTemplate(
		EEventsTemplate TemplateType, const FString& InExperimentId);

	// Finish the pending events at the current time
	void FinishPendingEvents(const float EndTime);

	// Directory where to log
	FString LogDirectory;

	// Unique id of the episode
	FString EpisodeId;

	// Type of owl template to write the events to
	EEventsTemplate OwlDocTemplate;

	// Array of pending events
	TArray<IEvent*> PendingEvents;

	// Array of pending events
	TArray<IEvent*> FinishedEvents;

	// Owl document of the finished events
	TSharedPtr<FOwlEvents> EventsDoc;
};