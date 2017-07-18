# USemLog

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.

# Actor tags example:

	SemLog;Class,HelaCurryKetchup;LogType,Dynamic;

# Development branch

	Unreal editor mode
	
# Code snippets

## Broadcast and read published raw data:

 * enable `bBroadcastRawData` in your `ASLRuntimeManager`;
 * create a new actor where to listen to the events in this case `ARawDataDelegateListener`:
 

.h 
``` 
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
	UFUNCTION()
	void OnReceiveRawData(const FString& NewData);
};

```

.cpp
```
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
			// Bind the new data envents
			RawDataLogger->OnNewData.AddUObject(this,
				&ARawDataDelegateListener::OnReceiveRawData);
		}
	}
}

void ARawDataDelegateListener::OnReceiveRawData(const FString& RawData)
{
	UE_LOG(LogTemp, Warning, TEXT("Data: %s"), *RawData);
}
```
 