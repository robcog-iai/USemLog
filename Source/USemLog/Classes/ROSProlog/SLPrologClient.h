// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "Tickable.h"
#include "CoreMinimal.h"

#if SL_WITH_ROSBRIDGE
#include "ROSBridgeHandler.h"
#include "rosprolog_msgs/Query.h"
#include "rosprolog_msgs/NextSolution.h"
#include "rosprolog_msgs/Finish.h"
#include "ROSProlog/SLROSServiceClient.h"
#endif // SL_WITH_ROSBRIDGE

#include "ROSProlog/SLQueryHandler.h"
#include "UObject/NoExportTypes.h"
#include "Events/ISLEventHandler.h"
#include "SLPrologClient.generated.h"

/**
 * ROS Prolog Client to log and query into Knowrob
 */
UCLASS()
class USEMLOG_API USLPrologClient : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	// Constructor
	USLPrologClient();

	// Destructor
	~USLPrologClient();

#if SL_WITH_ROSBRIDGE
	// Disconnect from ROSBridge
	void Disconnect();

	// Init socket connection
	void Init(FString Host, uint32 port);

	// Add query to buffer
	void AddQuery(FString Id, FSLQueryHandler *QueryHandle);

	// Add event query
	void AddEventQuery(TSharedPtr<ISLEvent> Event);

	// Add an add object query
	void AddObjectQuery(FSLEntity *Entity);

	// Send query <Id>
	void SendPrologQuery(FString Id);

	// Request next solution to query <Id>
	void SendNextSolutionCommand(FString Id);

	// Finsh query <Id>
	void SendFinishCommand(FString Id);

	// Process response pipeline
	void ProcessResponse(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse, FString Type);
#endif // SL_WITH_ROSBRIDGE

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

#if SL_WITH_ROSBRIDGE
	// ROS Connection handler
	TSharedPtr<FROSBridgeHandler> ROSHandler;

	// Service clients for every command
	TSharedPtr<SLROSServiceClient> ROSPrologQueryClient;
	TSharedPtr<SLROSServiceClient> ROSPrologNextSolutionClient;
	TSharedPtr<SLROSServiceClient> ROSPrologFinishClient;

	// Keeping record of sent commands and their Ids
	TMap<TSharedPtr<FROSBridgeSrv::SrvResponse>, FString> SentQueries;
	TMap<TSharedPtr<FROSBridgeSrv::SrvResponse>, FString> SentNextSolutionCommands;
	TMap<TSharedPtr<FROSBridgeSrv::SrvResponse>, FString> SentFinishCommands;
#endif // SL_WITH_ROSBRIDGE

	// Record of active queries
	TMap<FString, FSLQueryHandler*> Queries;

	// Buffers for every command
	TArray<FString> QueriesBuffer;
	TArray<FString> NextSolutionCommandsBuffer;
	TArray<FString> FinishCommandsBuffer;

};
