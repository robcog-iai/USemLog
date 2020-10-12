// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include "Knowrob/Proto/knowrob_ameva.pb.h"
#include "CoreMinimal.h"

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
	void ProcessProtobuf(std::string ProtoStr);

private:
	// Set the task of MongoManager
	void SetTask(sl_pb::SetTaskParams params);

	// Set the episode of MongoManager
	void SetEpisode(sl_pb::SetEpisodeParams params);

	// Draw the trajectory
	void DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params);

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
};
