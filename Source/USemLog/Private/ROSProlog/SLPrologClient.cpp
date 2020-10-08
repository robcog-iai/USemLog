// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#include "ROSProlog/SLPrologClient.h"

// Constructor
USLPrologClient::USLPrologClient() 
{
	bIsTickable = true;
}

// Destructor
USLPrologClient::~USLPrologClient()
{
}

// Return if object is ready to be ticked
bool USLPrologClient::IsTickable() const
{
	return bIsTickable;
}

// Return the stat id to use for this tickable
TStatId USLPrologClient::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USLPrologClient, STATGROUP_Tickables);
}
/** End FTickableGameObject interface */


/** Begin FTickableGameObject interface */
// Called after ticking all actors, DeltaTime is the time passed since the last call.
void USLPrologClient::Tick(float DeltaTime) 
{
#if SL_WITH_ROSBRIDGE
	// Call update on tick
	while (QueriesBuffer.Num() > 0) 
	{
		SendPrologQuery(QueriesBuffer.Pop());
	}
	while (NextSolutionCommandsBuffer.Num() > 0) 
	{
		SendNextSolutionCommand(NextSolutionCommandsBuffer.Pop());
	}
	while (FinishCommandsBuffer.Num() > 0) 
	{
		SendFinishCommand(FinishCommandsBuffer.Pop());
	}
#endif
}

#if SL_WITH_ROSBRIDGE
void USLPrologClient::Init(FString Host, uint32 port) 
{

	// Defining ROSBridge handler
	ROSHandler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(Host, port));

	// Defining clients for different commands
	ROSPrologQueryClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/query", "query"));
	ROSPrologNextSolutionClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/next_solution", "next_solution"));
	ROSPrologFinishClient = MakeShareable<SLROSServiceClient>(new SLROSServiceClient(this, "/rosprolog/finish", "finish"));

	// Start Connection
	if (ROSHandler.IsValid()) 
	{
		ROSHandler->Connect();
		UE_LOG(LogTemp, Warning, TEXT("Connecting Prolog Client to %s:%d"), *Host, port);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("No FROSBridgeHandler created for this Prolog Client."));
	}
}

// Disconnect from ROSBridge
void USLPrologClient::Disconnect() 
{

	// Finish ROS Connection
	if (ROSHandler.IsValid())
	{
		ROSHandler->Disconnect();
	}
}

// Add query into buffer
void USLPrologClient::AddQuery(FString Id, FSLQueryHandler *QueryHandle) 
{
	Queries.Add(Id, QueryHandle);
	QueriesBuffer.Push(Id);
}

// Add event query
void USLPrologClient::AddEventQuery(TSharedPtr<ISLEvent> Event) 
{
	FSLQueryHandler* QueryHandler =  new FSLQueryHandler(Event->ToROSQuery(), false);
	AddQuery(Event->Id, QueryHandler);
}

// Add object query
void USLPrologClient::AddObjectQuery(FSLEntity *Entity) 
{
	
	// Creates query ID
	FString IdValue = FSLUuid::NewGuidInBase64();

	// Temporally remove rotations to get real bounding box
	AActor* Object = Cast<AActor>(Entity->Obj);
	FRotator OrigRot = Object->GetActorRotation();
	Object->SetActorRotation(FRotator(0, 0, 0));
	// Get Bounds
	FVector Extent = Object->GetComponentsBoundingBox().GetExtent();
	// Set original rotation
	Object->SetActorRotation(OrigRot);

	// Creates query
	FString Query = FString::Printf(TEXT("Obj = \'http://www.ease-crc.org/ont/SOMA.owl#%s_%s\', "), *Entity->Class, *Entity->Id);
	Query.Append("tell([");
	Query.Append("is_object(Obj), ");
	Query.Append(FString::Printf(TEXT("has_type(Shape, soma:\'Shape\'), ")));
	Query.Append(FString::Printf(TEXT("has_type(ShapeReg, soma:\'BoxShape\'), ")));
	Query.Append(FString::Printf(TEXT("holds(ShapeReg, soma:hasDepth, %f), "), Extent.X));
	Query.Append(FString::Printf(TEXT("holds(ShapeReg, soma:hasWidth, %f), "), Extent.Y));
	Query.Append(FString::Printf(TEXT("holds(ShapeReg, soma:hasHeight, %f), "), Extent.Z));
	Query.Append(FString::Printf(TEXT("holds(Shape, dul:hasRegion, ShapeReg), ")));
	Query.Append(FString::Printf(TEXT("holds(Obj, soma:hasShape, Shape) ")));
	Query.Append("]).");

	FSLQueryHandler* QueryHandler = new FSLQueryHandler(Query, false);
	AddQuery(IdValue, QueryHandler);
}

// Send query 
void USLPrologClient::SendPrologQuery(FString Id) 
{

	FSLQueryHandler* QueryHandler = Queries.FindRef(Id);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Query::Request(0, QueryHandler->Query, Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Query::Response());
	ROSHandler->CallService(ROSPrologQueryClient, Request, Response);
	SentQueries.Add(Response, Id);
}

// Request next solution 
void USLPrologClient::SendNextSolutionCommand(FString Id) 
{

	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::NextSolution::Request(Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::NextSolution::Response());
	ROSHandler->CallService(ROSPrologNextSolutionClient, Request, Response);
	SentNextSolutionCommands.Add(Response, Id);
}

// Finish query
void USLPrologClient::SendFinishCommand(FString Id) 
{

	Queries.Remove(Id);
	TSharedPtr<FROSBridgeSrv::SrvRequest> Request = MakeShareable<FROSBridgeSrv::SrvRequest>(new rosprolog_msgs::Finish::Request(Id));
	TSharedPtr<FROSBridgeSrv::SrvResponse> Response = MakeShareable<FROSBridgeSrv::SrvResponse>(new rosprolog_msgs::Finish::Response());
	ROSHandler->CallService(ROSPrologFinishClient, Request, Response);
	SentFinishCommands.Add(Response, Id);
}

// Process response from commands
void  USLPrologClient::ProcessResponse(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse, FString Type) 
{
	
	FString Id;
	
	if (Type.Equals("query")) 
	{
		TSharedPtr<rosprolog_msgs::Query::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::Query::Response>(InResponse);
		bool bSuccess = Msg->GetSuccess();
		if (!SentQueries.RemoveAndCopyValue(InResponse, Id)) {
			UE_LOG(LogTemp, Warning, TEXT("No previous Query found for this response: OK = %s; Message = \"%s\""), \
				*Id, bSuccess ? TEXT("true") : TEXT("false"), *Msg->GetMessage());
		}
		if (bSuccess) {
			SendNextSolutionCommand(Id);
		}
	}

	if (Type.Equals("next_solution")) 
	{
		
		TSharedPtr<rosprolog_msgs::NextSolution::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::NextSolution::Response>(InResponse);
		int status = Msg->GetStatus();

		if (!SentNextSolutionCommands.RemoveAndCopyValue(InResponse, Id)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("No previous Next_Solution Command found for this response: Status = %d; Solution = \"%s\""), \
				*Id, status, *Msg->GetSolution());
		}
	
		// Handling Solutions
		FSLQueryHandler* QueryHandler = Queries.FindRef(Id);
		QueryHandler->SolutionsCount++;

		if (QueryHandler) 
		{
			// Callback
			QueryHandler->SolutionCallback(status, Msg->GetSolution());

			// Gettig next solution or finishing
			if (!QueryHandler->bExhaustSolutions || !status || QueryHandler->SolutionsCount >= QueryHandler->SolutionsLimit) 
			{
				SendFinishCommand(Id);
			}
			else 
			{
				SendNextSolutionCommand(Id);
			}
		}
		else 
		{
			SendFinishCommand(Id);
		}
	}

	if (Type.Equals("finish")) 
	{
		TSharedPtr<rosprolog_msgs::Finish::Response> Msg = StaticCastSharedPtr<rosprolog_msgs::Finish::Response>(InResponse);
		if (!SentFinishCommands.RemoveAndCopyValue(InResponse, Id)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("Response for Finish Command received. No previous finish command found."));
		}
	}
}
#endif // SL_WITH_ROSBRIDGE