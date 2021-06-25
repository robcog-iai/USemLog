// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQReplay.h"
#include "Knowrob/SLKnowrobManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLVizQReplay::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQReplay, bPauseRadioButton))
	{
		if (IsReadyForManualExecution())
		{
			KnowrobManager->GetVizManager()->PauseReplay(bPauseRadioButton);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQReplay, bStopButton))
	{
		bStopButton = false;
		if (IsReadyForManualExecution())
		{
			KnowrobManager->GetVizManager()->StopReplay();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQReplay, StartTime))
	{
		if (bLiveUpdate && Type == ESLVizQReplayType::Goto)
		{
			if (IsReadyForManualExecution())
			{
				KnowrobManager->GetVizManager()->GotoCachedEpisodeFrame(Episode, StartTime);
			}
		}
	}
}
#endif // WITH_EDITOR

// Virtual implementation of the execute function
void USLVizQReplay::ExecuteImpl(ASLKnowrobManager* KRManager)
{
	ASLVizManager* VizManager = KRManager->GetVizManager();
	ASLMongoQueryManager* MongoQueryManager = KRManager->GetMongoQueryManager();

	// Retrieve and cache episode
	if (!VizManager->IsEpisodeCached(Episode))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Collecting episode %s::%s .."),
			*FString(__FUNCTION__), __LINE__, *Task, *Episode);
		auto EpisodeData = MongoQueryManager->GetEpisodeData(Task, Episode);
		if (!VizManager->CacheEpisodeData(Episode, EpisodeData))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not cache episode %s::%s, execution aborted .."),
				*FString(__FUNCTION__), __LINE__, *Task, *Episode);
			return;
		}
	}

	// Execute task
	if (Type == ESLVizQReplayType::Goto)
	{
		VizManager->GotoCachedEpisodeFrame(Episode, StartTime);		
	}
	else if (Type == ESLVizQReplayType::Replay)
	{
		FSLVizEpisodePlayParams Params;
		Params.StartTime = StartTime;
		Params.EndTime = EndTime;
		Params.bLoop = bLoop;
		Params.UpdateRate = UpdateRate;
		Params.StepSize = StepSize;
		VizManager->ReplayCachedEpisode(Episode, Params);
	}
}
