// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "World/SLWorldAsyncWorker.h"
#include "Tickable.h"
#include "SLWorldLogger.generated.h"

/**
 * Raw (subsymbolic) data logger, 
 * it synchronizes(ticks) the async worker on saving the world state at given timepoints.
 * Inherit from FTickableGameObject to have it's own tick
 */
UCLASS()
class USEMLOG_API USLWorldLogger : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLWorldLogger();

	// Destructor
	~USLWorldLogger();

	// Init Logger
	void Init(ESLWorldWriterType WriterType, const FSLWorldWriterParams& InWriterParams);

	// Start logger
	void Start(const float UpdateRate);

	// Finish logger
	void Finish(bool bForced = false);

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
	// Log initial state of the world (static and dynamic entities)
	void InitialUpdate();

	// Log current state of the world (dynamic objects that moved more than the distance threshold)
	void Update();

	// Delay function to set tick to true (avoid logging first frame twice)
	void DelaySetTickTrue();

private:
	// Set when logger is initialized
	bool bIsInit;

	// Set when logger is started
	bool bIsStarted;

	// Set when logger is finished
	bool bIsFinished;

	// True if the object can be ticked (used by FTickableGameObject)
	bool bIsTickable;

	// Timer handle for custom update rate
	FTimerHandle TimerHandle;

	// Async worker to log the raw data on a separate thread
	FAsyncTask<FSLWorldAsyncWorker>* AsyncWorker;
};
