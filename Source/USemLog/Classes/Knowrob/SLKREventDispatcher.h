// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include "Knowrob/Proto/ameva.pb.h"
#include "CoreMinimal.h"

class ASLMongoQueryManager;
class ASLVizManager;
class ASLControlManager;
class ASLSemanticMapManager;
class ASLSymbolicLogger;
class ASLIndividualManager;

enum class ESLVizPrimitiveMarkerType : uint8;
enum class ESLVizMaterialType : uint8;
/**
 * 
 */
class USEMLOG_API FSLKREventDispatcher
{
public:
	// Default ctor
	FSLKREventDispatcher(ASLMongoQueryManager* InMongoManger, ASLVizManager* InVizManager, ASLSemanticMapManager* InSemanticMapManager, ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger, ASLIndividualManager* IndividualManager);
	
	// Dector
	~FSLKREventDispatcher();

public:
	// Parse the proto sequence and trigger function
	FString ProcessProtobuf(std::string ProtoStr);

private:
	// Load the Semantic Map
	FString LoadMap(sl_pb::LoadMapParams params);
	
	// Set the task of MongoManager
	FString SetTask(sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	FString SetEpisode(sl_pb::SetEpisodeParams params);

	// Draw the trajectory
	FString DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params);

	// Start Symbolic Logger
	FString StartSymbolicLogger(sl_pb::StartSymbolicLogParams params);

	// Stop Symbolic Logger
	FString StoptSymbolicLogger();

	// Start Simulation
	FString StartSimulation(sl_pb::StartSimulationParams params);

	// Stop Simulation
	FString StopSimulation(sl_pb::StopSimulationParams params);

private:
	// -----  helper function  ------//
	// Transform the maker type
	ESLVizPrimitiveMarkerType GetMarkerType(sl_pb::MarkerType Marker);
	
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

	// Keeps access to all the individuals in the world
	ASLIndividualManager* IndividualManager;
};
