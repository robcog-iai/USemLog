// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQBase.h"
#include "Viz/SLVizManager.h"
#include "Mongo/SLMongoQueryManager.h"

// Base
void USLVizQBase::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	for (const auto& C : Children)
	{
		C->Execute(VizManager, MongoManager);
	}
}

// Goto
void USLVizQGotoFrame::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	VizManager->GotoCachedEpisodeFrame(EpisodeId, Timestamp);

	Super::Execute(VizManager, MongoManager);
}

// Play
void USLVizQReplayEpisode::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	FSLVizEpisodePlayParams Params;
	Params.StartTime = StartTime;
	Params.EndTime = EndTime;
	Params.bLoop = bLoop;
	Params.UpdateRate = UpdateRate;
	Params.StepSize = StepSize;
	VizManager->ReplayCachedEpisode(EpisodeId, Params);

	Super::Execute(VizManager, MongoManager);
}

// Episode
void USLVizQCacheEpisodes::Execute(ASLVizManager* VizManager, ASLMongoQueryManager* MongoManager)
{
	MongoManager->SetTask(TaskId);
	for (const auto& EpId : EpisodeIds)
	{
		TArray<TPair<float, TMap<FString, FTransform>>> EpisodeData = MongoManager->GetEpisodeData(EpId);
		VizManager->CacheEpisodeData(EpId, EpisodeData);
	}

	Super::Execute(VizManager, MongoManager);
}