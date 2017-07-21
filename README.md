# USemLog - development branch

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.
	
# Usage:

#### Include the plugin to your project
* Add the plugin to your project (e.g `MyProject/Plugins/USemLog`)

#### Tag semantic components (see [UTags](https://github.com/robcog-iai/UTags) for more details)

* Tag your actors / components that you want to semantically log:

  * Tag example:

    [`SemLog;Class,HelaCurryKetchup;Runtime,Dynamic;Id,gPP9;`]

    where

     `SemLog;` is the `TagType`
	 
     `Class,HelaCurryKetchup;` - represents the semantic class of the object
	 
     `Runtime,Dynamic;` - represents the raw logging type of the object (static or dynamic)
	 
     `Id,gPP9;` - represents the unique ID for each entity to be logged


# Tutorials

#### How to broadcast and read published raw and events data:

 * Add the module dependency to your module (Project/Plugin); In the `MyModule.Build.cs` file:  

		PublicDependencyModuleNames.AddRange(  
		new string[]  
		{  
		...  
		"SemLog",  
		...  
		}  
		);  
 
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
	
	// Iterate the runtime managers from the world
	for (TActorIterator<ASLRuntimeManager>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		// Get the raw data logger
		if (USLRawDataLogger* RawDataLogger = (*RMItr)->GetRawDataLogger())
		{
			// Listen for new data
			RawDataLogger->OnNewData.AddUObject(this,
				&ARawDataDelegateListener::OnReceiveRawData);
		}

		// Get the event data logger
		if (USLEventDataLogger* EventDataLogger = (*RMItr)->GetEventDataLogger())
		{
			// Listen for when the events are finished
			EventDataLogger->OnEventsFinished.AddUObject(this,
				&ARawDataDelegateListener::OnReceiveFinishedEventsData);
		}
		// Break loop, there should only be one manager
		break;
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
 