// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "SLROSPrologLogger.h"

USLROSPrologLogger::USLROSPrologLogger() {
	bIsTickable = true;
}

USLROSPrologLogger::~USLROSPrologLogger(){
}

/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLROSPrologLogger::Tick(float DeltaTime)
{
	// Call update on tick
	UE_LOG(LogTemp, Warning, TEXT("Jo JO JKO"));
}

// Return if object is ready to be ticked
bool USLROSPrologLogger::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLROSPrologLogger::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLROSPrologLogger, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */

void USLROSPrologLogger::Connect(FString Host, uint32 port) {
	// Start ROS Connection
	ROSHandler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(Host, port));
	if (ROSHandler.IsValid()) {
		ROSHandler->Connect();
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created for Event Logger."));
	}
}

void USLROSPrologLogger::Disconnect() {
	// Finish ROS Connection
	if (ROSHandler.IsValid())
	{
		ROSHandler->Disconnect();
	}
}
  
void USLROSPrologLogger::AddEvent(TSharedPtr<ISLEvent> Event) {
	QueriesBuffer.Add(Event->Id, Event->ToROSQuery());
}

void USLROSPrologLogger::SendPrologQuery(FString Id) {
	
	FString Query;
	QueriesBuffer.RemoveAndCopyValue(Id, Query);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Query::Request(0, Query, Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Query::Response());
	ROSHandler->CallService(ROSClient, Request, Response);

}