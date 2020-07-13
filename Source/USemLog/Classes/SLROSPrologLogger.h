// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "Tickable.h"
#include "CoreMinimal.h"
#include "std_msgs/String.h"
#include "ROSBridgeHandler.h"
#include "SLROSServiceClient.h"
#include "rosprolog_msgs/Query.h"
#include "UObject/NoExportTypes.h"
#include "Events/ISLEventHandler.h"
#include "SLROSPrologLogger.generated.h"

/**
 * Raw (event) data logger,
 * it synchronizes(ticks) the async worker on saving the world state at given timepoints.
 * Inherit from FTickableGameObject to have it's own tick
 */
UCLASS()
class USEMLOG_API USLROSPrologLogger : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	// Constructor
	USLROSPrologLogger();

	// Destructor
	~USLROSPrologLogger();

	void Disconnect();
	void Connect(FString Host, uint32 port);

	void AddEvent(TSharedPtr<ISLEvent> Event);

	void SendPrologQuery(FString Id);

protected:

	/** Begin FTickableGameObject interface */
	// Called after ticking all actors, DeltaTime is the time passed since the last call.
	virtual void Tick(float DeltaTime) override;

	// Return if object is ready to be ticked
	virtual bool IsTickable() const override;

	// Return the stat id to use for this tickable
	virtual TStatId GetStatId() const override;
	/** End FTickableGameObject interface */

private:

	// True if the object can be ticked (used by FTickableGameObject)
	bool bIsTickable;

	// ROS Connection handlers
	TSharedPtr<FROSBridgeHandler> ROSHandler;
	TSharedPtr<SLROSServiceClient> ROSClient;

	// Query Queue
	TMap<FString, FString> QueriesBuffer;
};
