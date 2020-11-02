// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQBase.h"
#include "Mongo/SLMongoQueryManager.h"

// Base
void USLVizQBase::Execute(ASLKnowrobManager* KRManager)
{
	for (const auto& C : Children)
	{
		C->Execute(KRManager);
	}
}

// Goto
void USLVizQGotoFrame::Execute(ASLKnowrobManager* KRManager)
{
	//VizManager->GotoCachedEpisodeFrame(EpisodeId, Timestamp);

	Super::Execute(KRManager);
}

// Play
void USLVizQReplayEpisode::Execute(ASLKnowrobManager* KRManager)
{
	//FSLVizEpisodePlayParams Params;
	//Params.StartTime = StartTime;
	//Params.EndTime = EndTime;
	//Params.bLoop = bLoop;
	//Params.UpdateRate = UpdateRate;
	//Params.StepSize = StepSize;
	//VizManager->ReplayCachedEpisode(EpisodeId, Params);

	Super::Execute(KRManager);
}

// Episode
void USLVizQCacheEpisodes::Execute(ASLKnowrobManager* KRManager)
{
	//MongoManager->SetTask(TaskId);
	//for (const auto& EpId : EpisodeIds)
	//{
	//	TArray<TPair<float, TMap<FString, FTransform>>> EpisodeData = MongoManager->GetEpisodeData(EpId);
	//	VizManager->CacheEpisodeData(EpId, EpisodeData);
	//}

	Super::Execute(KRManager);
}