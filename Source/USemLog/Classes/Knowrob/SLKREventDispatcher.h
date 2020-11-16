// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include <string>
#if SL_WITH_PROTO_MSGS
#include "Proto/ameva.pb.h"
#endif // SL_WITH_PROTO_MSGS	
#include "Runtime/SLLoggerStructs.h"
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
	FSLKREventDispatcher(ASLMongoQueryManager* InMongoManger, ASLVizManager* InVizManager, ASLSemanticMapManager* InSemanticMapManager, ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger);
	
	// Dector
	~FSLKREventDispatcher();

public:
	// Parse the proto sequence and trigger function
	void ProcessProtobuf(FSLKRResponse& Out, std::string ProtoStr);

private:
#if SL_WITH_PROTO_MSGS
	// Load the Semantic Map
	void LoadMap(FSLKRResponse& Out, sl_pb::LoadMapParams params);

	// Set the task of MongoManager
	void SetTask(FSLKRResponse& Out, sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	void SetEpisode(FSLKRResponse& Out, sl_pb::SetEpisodeParams params);

	// Draw the trajectory
	void DrawMarkerTraj(FSLKRResponse& Out, sl_pb::DrawMarkerTrajParams params);

	// Start Symbolic Logger
	void StartSymbolicLogger(FSLKRResponse& Out, sl_pb::StartSymbolicLogParams params);

	// Stop Symbolic Logger
	void StoptSymbolicLogger(FSLKRResponse& Out);

	// Start Simulation
	void StartSimulation(FSLKRResponse& Out, sl_pb::StartSimulationParams params);

	// Stop Simulation
	void StopSimulation(FSLKRResponse& Out, sl_pb::StopSimulationParams params);

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

	// Logger parameters
	FSLSymbolicLoggerParams LoggerParameters;

	// Location parameters
	FSLLoggerLocationParams LocationParameters;
};
