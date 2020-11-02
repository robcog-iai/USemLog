// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include <string>
#if SL_WITH_PROTO_MSGS
#include "Proto/ameva.pb.h"
#endif // SL_WITH_PROTO_MSGS	
#include "CoreMinimal.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;
enum class ESLVizPrimitiveMarkerType : uint8;
enum class ESLVizMaterialType : uint8;

/**
 * 
 */
class USEMLOG_API FSLKREventDispatcher
{
public:
	// Default ctor
	FSLKREventDispatcher(ASLMongoQueryManager* InMongoManger, ASLVizManager* InVizManager);
	
	// Dector
	~FSLKREventDispatcher();

public:
	// Parse the proto sequence and trigger function
	FString ProcessProtobuf(std::string ProtoStr);

private:
#if SL_WITH_PROTO_MSGS
	// Set the task of MongoManager
	FString SetTask(sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	FString SetEpisode(sl_pb::SetEpisodeParams params);

	// Draw the trajectory
	FString DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params);

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
};
