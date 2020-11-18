// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include <string>
#define SL_WITH_PROTO_MSGS 1
#if SL_WITH_PROTO_MSGS
#include "Proto/ameva.pb.h"
#endif // SL_WITH_PROTO_MSGS	
#include "Runtime/SLLoggerStructs.h"
#include "Knowrob/SLKRWSClient.h"
#include "SLKRResponseStruct.h"
#include "CoreMinimal.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;
class ASLControlManager;
class ASLSemanticMapManager;
class ASLSymbolicLogger;

enum class ESLVizPrimitiveMarkerType : uint8;
enum class ESLVizMaterialType : uint8;

/**
 * 
 */
class USEMLOG_API FSLKREventDispatcher
{
public:
	// Default ctor
	FSLKREventDispatcher(TSharedPtr<FSLKRWSClient> InKRWSClient, UWorld* InWorld, ASLMongoQueryManager* InMongoManger, ASLVizManager* InVizManager, ASLSemanticMapManager* InSemanticMapManager, ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger);
	
	// Dector
	~FSLKREventDispatcher();

public:
	// Parse the proto sequence and trigger function
	void ProcessProtobuf(std::string ProtoStr);

private:
#if SL_WITH_PROTO_MSGS
	// Load the Semantic Map
	void LoadMap(sl_pb::LoadMapParams params);

	// Set the task of MongoManager
	void SetTask(sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	void SetEpisode(sl_pb::SetEpisodeParams params);

	// Draw the trajectory
	void DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params);

	// Start Symbolic Logger
	void StartSymbolicLogger(sl_pb::StartSymbolicLogParams params);

	// Stop Symbolic Logger
	void StopSymbolicLogger();

	// Start Simulation
	void StartSimulation(sl_pb::StartSimulationParams params);

	// Stop Simulation
	void StopSimulation(sl_pb::StopSimulationParams params);

	// Move Individual
	void MoveIndividual(sl_pb::MoveIndividualParams params);

	// Start Symbolic logging and simulation for seconds
	void SimulateAndLogForSeconds(sl_pb::SimulateAndLogForSecondsParams params);

private:
	// -----  helper function  ------//
	// Transform the maker type
	ESLVizPrimitiveMarkerType GetMarkerType(sl_pb::MarkerType Marker);
#endif // SL_WITH_PROTO_MSGS	
	// Transform the string to color
	FLinearColor GetMarkerColor(const FString& Color);

	// Transform the material type
	ESLVizMaterialType GetMarkerMaterialType(const FString& MaterialType);


private:
	// Used to query the subsymbolic data from mongo
	ASLMongoQueryManager* MongoManager;

	// Used to visualize the world using various markers
	ASLVizManager* VizManager;
	
	// Used to switch Semantic Map
	ASLSemanticMapManager* SemanticMapManager;

	// Used to control individual
	ASLControlManager* ControlManager;
	
	// Used for symbolic logging
	ASLSymbolicLogger* SymbolicLogger;

	// Used for sending response 
	TSharedPtr<FSLKRWSClient> KRWSClient;

	// Used for keep the current world
	UWorld* World;

	// Logger parameters
	FSLSymbolicLoggerParams LoggerParameters;

	// Location parameters
	FSLLoggerLocationParams LocationParameters;
};
