// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "SLROSPrologLogger.h"

USLROSPrologLogger::USLROSPrologLogger() {
	bIsTickable = true;
}

USLROSPrologLogger::~USLROSPrologLogger(){
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


/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLROSPrologLogger::Tick(float DeltaTime) {
#if SL_WITH_ROSBRIDGE
	// Call update on tick
	if (QueriesBuffer.Num() > 0) {
		for (auto It = QueriesBuffer.CreateConstIterator(); It; ++It) {
			SendPrologQuery(It.Key());
		}
	}
	while (NextSolutionCommandsBuffer.Num() > 0) {
		SendNextSolutionCommand(NextSolutionCommandsBuffer.Pop());
	}
	while (FinishCommandsBuffer.Num() > 0) {
		SendFinishCommand(FinishCommandsBuffer.Pop());
	}
#endif
}

#if SL_WITH_ROSBRIDGE
void USLROSPrologLogger::Init(FString Host, uint32 port) {

	// Start ROS Connection
	ROSHandler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(Host, port));
	ROSPrologQueryClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/query", "query"));
	ROSPrologNextSolutionClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/next_solution", "next_solution"));
	ROSPrologFinishClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/finish", "finish"));

	if (ROSHandler.IsValid()) {
		//ROSHandler->
		ROSHandler->Connect();
		UE_LOG(LogTemp, Warning, TEXT("Connecting ROSPRolog Client to %s:%d"), *Host, port);
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
	UE_LOG(LogTemp, Warning, TEXT("Sending Query : ID = %s; Query = %s;"), *Id, *Query);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Query::Request(0, Query, Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Query::Response());
	ROSHandler->CallService(ROSPrologQueryClient, Request, Response);
	SentQueries.Add(Response, Id);
}

void USLROSPrologLogger::SendNextSolutionCommand(FString Id) {

	UE_LOG(LogTemp, Warning, TEXT("Sending Next Solution Command : ID = %s;"), *Id);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::NextSolution::Request(Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::NextSolution::Response());
	ROSHandler->CallService(ROSPrologNextSolutionClient, Request, Response);
	SentNextSolutionCommands.Add(Response, Id);
}

void USLROSPrologLogger::SendFinishCommand(FString Id) {

	UE_LOG(LogTemp, Warning, TEXT("Sending Finish Command: ID = %s;"), *Id);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Finish::Request(Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Finish::Response());
	ROSHandler->CallService(ROSPrologFinishClient, Request, Response);
	SentFinishCommands.Add(Response, Id);
}

void  USLROSPrologLogger::ProcessResponse(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse, FString Type) {
	
	FString Id;
	
	if (Type.Equals("query")) {
		TSharedPtr<rosprolog_msgs::Query::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::Query::Response>(InResponse);
		if (SentQueries.RemoveAndCopyValue(InResponse, Id)) {
			UE_LOG(LogTemp, Warning, TEXT("Response for Query: ID = %s; OK = %s; Message = \"%s\""), \
				*Id, Msg->GetSuccess()? TEXT("true") : TEXT("false"), *Msg->GetMessage());
			SendNextSolutionCommand(Id);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No previous Query found for this response: OK = %s; Message = \"%s\""), \
				*Id, Msg->GetSuccess() ? TEXT("true") : TEXT("false"), *Msg->GetMessage());
		}
	}

	if (Type.Equals("next_solution")) {
		TSharedPtr<rosprolog_msgs::NextSolution::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::NextSolution::Response>(InResponse);
		if (SentNextSolutionCommands.RemoveAndCopyValue(InResponse, Id)) {
			UE_LOG(LogTemp, Warning, TEXT("Response for Next_Solution Command: Id = %s; Status = %d; Solution = \"%s\""), \
				*Id, Msg->GetStatus(), *Msg->GetSolution());
			SendFinishCommand(Id);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No previous Next_Solution Command found for this response: Status = %d; Solution = \"%s\""), \
				*Id, Msg->GetStatus(), *Msg->GetSolution());
		}
	}

	if (Type.Equals("finish")) {
		TSharedPtr<rosprolog_msgs::Finish::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::Finish::Response>(InResponse);
		if (SentFinishCommands.RemoveAndCopyValue(InResponse, Id)) {
			UE_LOG(LogTemp, Warning, TEXT("Response for Finish Command received. Id = %s"), *Id);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Response for Finish Command received. No previous finish command found."));
		}
	}
}
#endif // SL_WITH_ROSBRIDGE