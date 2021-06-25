// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include <string>
#if SL_WITH_PROTO
#include "Proto/SLProtoMsgType.h"
#endif // SL_WITH_PROTO	
#include "Runtime/SLLoggerStructs.h"
#include "Knowrob/SLKRWSClient.h"
#include "SLKRResponseStruct.h"
#include "CoreMinimal.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;
class ASLControlManager;
class ASLLevelManager;
class ASLSymbolicLogger;
class ASLWorldStateLogger;

enum class ESLVizPrimitiveMarkerType : uint8;
enum class ESLVizMaterialType : uint8;

/**
 * 
 */
class USEMLOG_API SLKRMsgDispatcher
{
public:
	// Default ctor
	SLKRMsgDispatcher();
	
	// Dector
	~SLKRMsgDispatcher();

	// Set up required manager
	void Init(TSharedPtr<FSLKRWSClient> InKRWSClient, ASLMongoQueryManager* InMongoManger, 
		ASLVizManager* InVizManager, ASLLevelManager* InLevelManager, 
		ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger, 
		ASLWorldStateLogger* InWorldStateLogger, const FString InMongoSrvIP, int32 InMongoSrvPort);

	// Check if the manager is set
	bool IsInit() const { return bIsInit; }
	
	// Reset setting
	void Reset();

public:
	// Parse the proto sequence and trigger function
	void ProcessProtobuf(std::string ProtoStr);

private:
#if SL_WITH_PROTO
	// Load the level 
	void LoadLevel(sl_pb::LoadLevelParams params);

	// Set the task of MongoManager
	void SetTask(sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	void SetEpisode(sl_pb::SetEpisodeParams params);
	
	// Draw the individual marker
	void DrawMarker(sl_pb::DrawMarkerAtParams);

	// Draw the individual trajectory
	void DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params);

	// Hightlight the individual
	void HighlightIndividual(sl_pb::HighlightParams params);

	// Remove the individual hightlight
	void RemoveIndividualHighlight(sl_pb::RemoveHighlightParams params);

	// Hightlight the individual
	void RemoveAllIndividualHighlight();

	// Start Symbolic and World State Logger
	void StartLogging(sl_pb::StartLoggingParams params);

	// Stop Symbolic and World Logger
	void StopLogging();

	// Send the Episode data
	void SendEpisodeData(sl_pb::GetEpisodeDataParams params);

	// Start Simulation
	void StartSimulation(sl_pb::StartSimulationParams params);

	// Stop Simulation
	void StopSimulation(sl_pb::StopSimulationParams params);

	// Set the pose of the idividual
	void SetIndividualPose(sl_pb::SetIndividualPoseParams params);
	
	// Apply force to individual
	void ApplyForceTo(sl_pb::ApplyForceToParams params);

private:
	// -----  helper function  ------//
	// Transform the maker type
	ESLVizPrimitiveMarkerType GetMarkerType(sl_pb::MarkerType Marker);
#endif // SL_WITH_PROTO	

	// Transform the string to color
	FLinearColor GetMarkerColor(const FString& Color);

	// Transform the material type
	ESLVizMaterialType GetMarkerMaterialType(const FString& MaterialType);

	// Send response when simulation start
	void SimulationStartResponse();

	// Send response when simulation stop
	void SimulationStopResponse();

private:
	// Used to query the subsymbolic data from mongo
	ASLMongoQueryManager* MongoManager;

	// Used to visualize the world using various markers
	ASLVizManager* VizManager;
	
	// Used to switch Semantic Map
	ASLLevelManager* LevelManager;

	// Used to control individual
	ASLControlManager* ControlManager;
	
	// Used for symbolic logging
	ASLSymbolicLogger* SymbolicLogger;

	// Used for symbolic logging
	ASLWorldStateLogger* WorldStateLogger;

	// Used for sending response 
	TSharedPtr<FSLKRWSClient> KRWSClient;

	// Mongo server ip addres
	FString MongoServerIP;

	// Knowrob server port
	int32 MongoServerPort;

	// True if the manager is initialized
	bool bIsInit;

};
