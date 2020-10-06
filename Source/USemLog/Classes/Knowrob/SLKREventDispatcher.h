// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include "Knowrob/Proto/knowrob_ameva.pb.h"
#include "CoreMinimal.h"

class ASLMongoQueryManager;
class ASLVizManager;
enum class ESLVizMeshType : uint8;

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
	// Print the set task protobuf message for testing
	void LogSetTask(sl_pb::SetTaskParams params);

	// Print the draw marker protobuf message for testing
	void LogDrawMarker(sl_pb::DrawMarkerAtParams params);

	// Transform the maker type
	FString GetVizMeshType(sl_pb::MarkerType Marker);
	
	// Transform the string to color
	FLinearColor GetMarkerColor(FString Color);
private:
	// Used to query the subsymbolic data from mongo
	ASLMongoQueryManager* MongoManager;

	// Used to visualize the world using various markers
	ASLVizManager* VizManager;
};
