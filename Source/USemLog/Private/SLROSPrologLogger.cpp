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
#if SL_WITH_ROSBRIDGE
	// Call update on tick
	if (QueriesBuffer.Num() > 0) {
		for (auto It = QueriesBuffer.CreateConstIterator(); It; ++It) {
			SendPrologQuery(It.Key());
		}
		UE_LOG(LogTemp, Warning, TEXT("Jo JO JKO"));
	}

	if (ROSHandler.IsValid())
	{
		ROSHandler->Process();
	}
#endif // SL_WITH_ROSBRIDGE

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

void USLROSPrologLogger::Init(FString Host, uint32 port) {
#if SL_WITH_ROSBRIDGE
	// Start ROS Connection
	ROSHandler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(Host, port));
	ROSClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient("/rosprolog/query","/rosprolog/query"));
	if (ROSHandler.IsValid()) {
		//ROSHandler->
		ROSHandler->Connect();
		UE_LOG(LogTemp, Warning, TEXT("Connecting ROSPRolog Client to %s:%d"), *Host, port);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created for Event Logger."));
	}
#endif 
}

void USLROSPrologLogger::Disconnect() {
#if SL_WITH_ROSBRIDGE
	// Finish ROS Connection
	if (ROSHandler.IsValid())
	{
		ROSHandler->Disconnect();
	}
#endif
}
  
void USLROSPrologLogger::AddEvent(TSharedPtr<ISLEvent> Event) {
	QueriesBuffer.Add(Event->Id, Event->ToROSQuery());
}

void USLROSPrologLogger::SendPrologQuery(FString Id) {
#if SL_WITH_ROSBRIDGE
	FString Query;
	QueriesBuffer.RemoveAndCopyValue(Id, Query);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Query::Request(0, Query, Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Query::Response());
	ROSHandler->CallService(ROSClient, Request, Response);
#endif
}