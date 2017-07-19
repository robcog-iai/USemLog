# USemLog - development branch

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.

# Actor tags example:

	SemLog;Class,HelaCurryKetchup;LogType,Dynamic;
	
# Code snippets:

## Broadcast and read published raw and events data:

 * Add an `ASLRuntimeManager` actor to your world
 * Enable `bBroadcastRawData` and `bLogEventData` in your `ASLRuntimeManager`;
 * The example actor (`ARawDataDelegateListener`) below subscribes to the broadcasted events and prints them to the log:
 
 

.h 
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RawDataDelegateListener.generated.h"

UCLASS()
class ROBCOG_API ARawDataDelegateListener : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARawDataDelegateListener();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Called when new raw data is received
	UFUNCTION()
	void OnReceiveRawData(const FString& RawData);

	// Called when the event data is finished
	UFUNCTION()
	void OnReceiveFinishedEventsData(const FString& EventData);
};


```

.cpp
```cpp
#include "RawDataDelegateListener.h"
#include "SLRuntimeManager.h"
#include "EngineUtils.h"

// Sets default values
ARawDataDelegateListener::ARawDataDelegateListener()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ARawDataDelegateListener::BeginPlay()
{
	Super::BeginPlay();
	// Get the runtime manager
	if (ASLRuntimeManager* RM = *TActorIterator<ASLRuntimeManager>(GetWorld()))
	{
		// Get the raw data logger
		if (USLRawDataLogger* RawDataLogger = RM->GetRawDataLogger())
		{
			// Listen for new data
			RawDataLogger->OnNewData.AddUObject(this,
				&ARawDataDelegateListener::OnReceiveRawData);
		}

		// Get the event data logger
		if (USLEventDataLogger* EventDataLogger = RM->GetEventDataLogger())
		{
			// Listen for when the events are finished
			EventDataLogger->OnEventsFinished.AddUObject(this,
				&ARawDataDelegateListener::OnReceiveFinishedEventsData);
		}
	}
}

// Called when new raw data is received
void ARawDataDelegateListener::OnReceiveRawData(const FString& RawData)
{
	UE_LOG(LogTemp, Warning, TEXT("Data: %s"), *RawData);
}

// Called when the event data is finished
void ARawDataDelegateListener::OnReceiveFinishedEventsData(const FString& EventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Data: %s"), *EventData);
}
```
 