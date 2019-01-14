// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldState/SLWorldStateAsyncWorker.h"
#include "SLWorldStateLogger.generated.h"


/**
 * Raw (subsymbolic) data logger, 
 * it synchronizes(ticks) the async worker on saving the world state at given timepoints.
 * Inherit from FTickableGameObject to have it's own tick
 */
UCLASS()
class USEMLOG_API USLWorldStateLogger : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// Constructor
	USLWorldStateLogger();

	// Destructor
	~USLWorldStateLogger();

	// Init Logger
	void Init(ESLWorldStateWriterType WriterType,
		float LinearDistance,
		float AngularDistance,
		const FString& Location,
		const FString& EpisodeId,
		const FString& ServerIp = "",
		const uint16 ServerPort = 0);

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
	void FirstUpdate();

	// Log current state of the world (dynamic objects that moved more than the distance threshold)
	void Update();

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
	FAsyncTask<FSLWorldStateAsyncWorker>* AsyncWorker;

#if WITH_SL_VIS
	FString ReplayRecordingName;
#endif // WITH_SL_VIS
};
