// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQBase.h"
#include "Viz/SLVizManager.h"

// Base
void USLVizQBase::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d Nothing to execute.."), *FString(__FUNCTION__), __LINE__);
}

// Batch
void USLVizQBatch::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	for (const auto& C : Children)
	{
		C->Execute(VizManager, MongoManager);
	}
}

// Episode
void USLVizQEpisode::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	UE_LOG(LogTemp, Log, TEXT("%s::%d ID=%.."), *FString(__FUNCTION__), __LINE__, *EpisodeId);
}