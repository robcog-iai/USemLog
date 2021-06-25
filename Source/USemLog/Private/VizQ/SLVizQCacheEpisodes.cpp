// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQCacheEpisodes.h"
#include "Knowrob/SLKnowrobManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"


// Virtual implementation of the execute function
void USLVizQCacheEpisodes::ExecuteImpl(ASLKnowrobManager* KRManager)
{
	ASLVizManager* VizManager = KRManager->GetVizManager();
	ASLMongoQueryManager* MongoQueryManager = KRManager->GetMongoQueryManager();

	for (const auto Episode : Episodes)
	{
		if (!VizManager->IsEpisodeCached(Episode))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Collecting episode %s::%s .."),
				*FString(__FUNCTION__), __LINE__, *Task, *Episode);

			auto EpisodeData = MongoQueryManager->GetEpisodeData(Task, Episode);
			if (!VizManager->CacheEpisodeData(Episode, EpisodeData))
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not cache episode %s::%s, execution aborted .."),
					*FString(__FUNCTION__), __LINE__, *Task, *Episode);
			}
		}
	}
}
