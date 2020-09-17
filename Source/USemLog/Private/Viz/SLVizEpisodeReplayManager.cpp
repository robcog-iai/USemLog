// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizEpisodeReplayManager.h"
#include "Viz/SLVizEpisodeReplayUtils.h"

#include "Components/PoseableMeshComponent.h"

// Sets default values
ASLVizEpisodeReplayManager::ASLVizEpisodeReplayManager()
{
	// Allow ticking, disable it by default (used for episode replay)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bWorldSetAsVisualOnly = false;
	bEpisodeLoaded = false;
	bLoopReplay = false;
	bInActiveReplay = false;

	ReplayDefaultUpdateRate = 0.f;
	ActiveFrameIndex = INDEX_NONE;
	ReplayFirstFrameIndex = INDEX_NONE;
	ReplayLastFrameIndex = INDEX_NONE;
	ReplayStepSize = 1;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called every frame
void ASLVizEpisodeReplayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// Set update rate
	//if (LoggerParameters.UpdateRate > 0.f)
	//{
	//	SetActorTickInterval(LoggerParameters.UpdateRate);
	//}
	//SetActorTickEnabled(true);
}

// Set the whole world as a visual, disable physics, collisions, attachments, unnecesary components
void ASLVizEpisodeReplayManager::SetWorldAsVisualOnly()
{
	if (bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d World has already been set as visual only.."),
			*FString(__FUNCTION__), __LINE__);
		return;
	}

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	FSLVizEpisodeReplayUtils::SetActorsAsVisualsOnly(GetWorld());

	// Add a poseable mesh component clone to the skeletal actors
	FSLVizEpisodeReplayUtils::AddPoseablMeshComponentsToSkeletalActors(GetWorld());

	// Mark world as visual only
	bWorldSetAsVisualOnly = true;
}

// Load episode data
void ASLVizEpisodeReplayManager::LoadEpisode(const FSLVizEpisodeData& InEpisodeDataFull, const FSLVizEpisodeData& InEpisodeDataCompact)
{
	// Check if the data is valid
	if (!InEpisodeDataFull.IsValid() || !InEpisodeDataCompact.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid to load.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Stop any active replay
	if (bInActiveReplay)
	{
		// TODO reset indexes
		// Stop replay

		ActiveFrameIndex = INDEX_NONE;
		ReplayFirstFrameIndex = INDEX_NONE;
		ReplayLastFrameIndex = INDEX_NONE;
	}

	// Clear any previous episode
	EpisodeDataFull.Clear();
	EpisodeDataCompact.Clear();

	// Set the episode data
	EpisodeDataFull = InEpisodeDataFull;
	EpisodeDataCompact = InEpisodeDataCompact;

	UE_LOG(LogTemp, Warning, TEXT("%s::%d EpisodeFull:\tTsNum=%d;\tFramesNum=%d;\t\tEpisodeCompact:\tTsNum=%d;\tFramesNum=%d;"),
		*FString(__FUNCTION__), __LINE__,
		EpisodeDataFull.Timestamps.Num(), EpisodeDataFull.Frames.Num(),
		EpisodeDataCompact.Timestamps.Num(), EpisodeDataCompact.Frames.Num());	

	// Calculate a default update rate  
	CalcDefaultUpdateRate(256);

	// Mark the episode loaded flag to true
	bEpisodeLoaded = true;
}

// Set visual world as in the given frame 
void ASLVizEpisodeReplayManager::GotoFrame(int32 FrameIndex)
{
	if (!bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d World is not set as visual only.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (!bEpisodeLoaded)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d No episode is loaded.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
		
	if(!EpisodeDataFull.Frames.IsValidIndex(FrameIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Frame index is not valid, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	ApplyPoses(EpisodeDataFull.Frames[FrameIndex]);

	ActiveFrameIndex = FrameIndex;
}

// Set visual world as in the given timestamp (binary search for nearest index)
void ASLVizEpisodeReplayManager::GotoFrame(float Timestamp)
{
	GotoFrame(FSLVizEpisodeReplayUtils::BinarySearchLessEqual(EpisodeDataFull.Timestamps, Timestamp));
}


//
//// Goto timestamp, if timestamp is too large it goes to the last frame, if too small goes to the first frame
//void ASLVizEpisodeReplayManager::GoTo(float Timestamp)
//{
//	ActiveFrameIndex = FBinaryFindIndex::LessEqual(Timestamps, Timestamp);
//	if (Frames.IsValidIndex(ActiveFrameIndex))
//	{
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Found index [%ld] is not valid.."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
//	}
//}
//
//// Goto the next nth frame
//void ASLVizEpisodeReplayManager::Next(int32 StepSize, bool bLoop)
//{	
//	if (Frames.Num() < 1)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//
//	if (StepSize > Frames.Num() / 2)
//	{
//		StepSize = Frames.Num() / 2;
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
//	}
//
//	if (ActiveFrameIndex < Frames.Num() - StepSize)
//	{
//		// Goto next step
//		ActiveFrameIndex += StepSize;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else if(bLoop)
//	{
//		// Goto first frame + the remaining steps
//		ActiveFrameIndex = Frames.Num() - 1 - ActiveFrameIndex;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply last frame (even if it is not a full step)
//		if (ActiveFrameIndex != Frames.Num() - 1)
//		{
//			ActiveFrameIndex = Frames.Num() - 1;
//			Frames[ActiveFrameIndex].ApplyPoses();
//		}
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
//	}
//}
//
//// Goto the previous nth frame
//void ASLVizEpisodeReplayManager::Previous(int32 StepSize, bool bLoop)
//{
//	if (Frames.Num() < 1)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//
//	if (StepSize > Frames.Num() / 2)
//	{
//		StepSize = Frames.Num() / 2;
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
//	}
//
//	if (ActiveFrameIndex - StepSize >= 0)
//	{
//		// Goto previous step
//		ActiveFrameIndex -= StepSize;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else if (bLoop)
//	{
//		// Goto last frame - the remaining steps
//		ActiveFrameIndex = Frames.Num() - 1 - ActiveFrameIndex;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply first frame (even if it is not a full step)
//		if (ActiveFrameIndex != 0)
//		{
//			ActiveFrameIndex = 0;
//			Frames[ActiveFrameIndex].ApplyPoses();
//		}
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached first frame [%ld].."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
//	}
//}
//
//// Replay all the episode with the given update rate (by default the update rate is calculated as the average of the first X frames)
//void ASLVizEpisodeReplayManager::Replay(int32 StepSize, float UpdateRate, bool bLoop)
//{
//
//
//	if (Frames.Num() < 2)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available to replay.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//	if (!GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
//	{
//		if (StepSize > Frames.Num() / 2)
//		{
//			StepSize = Frames.Num() / 2;
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
//		}
//
//		bLoopTheReplay = bLoop;
//		ReplayStepSize = StepSize;
//		ReplayFirstFrameIndex = 0;		
//		ReplayLastFrameIndex = Frames.Num() - 1;
//
//		// Apply first frame
//		ActiveFrameIndex = ReplayFirstFrameIndex;
//		Frames[ActiveFrameIndex].ApplyPoses();
//
//		if (UpdateRate < 0.f)
//		{
//			UpdateRate = CalcDefaultUpdateRate(8);
//		}
//
//		// Use delay since the first frame was already applied
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, 0.01, true, 0.01);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Previous replay was already running, restarting.."), *FString(__FUNCTION__), __LINE__);
//		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//		Replay(UpdateRate, bLoop);
//	}
//}
//
//// Replay the episode between the timestamp with the given update rate (by default the update rate is calculated as the average of the first X frames)
//void ASLVizEpisodeReplayManager::Replay(float StartTime, float EndTime, int32 StepSize, float UpdateRate, bool bLoop)
//{
//	if (Frames.Num() < 2)
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available to replay.."), *FString(__FUNCTION__), __LINE__);
//		return;
//	}
//	if (!GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
//	{
//		if (StepSize > Frames.Num() / 2)
//		{
//			StepSize = Frames.Num() / 2;
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
//		}
//
//		bLoopTheReplay = bLoop;
//		ReplayStepSize = StepSize;
//		ReplayFirstFrameIndex = FBinaryFindIndex::LessEqual(Timestamps, StartTime);		
//		ReplayLastFrameIndex = FBinaryFindIndex::LessEqual(Timestamps, EndTime);
//
//		// Apply first frame
//		ActiveFrameIndex = ReplayFirstFrameIndex;
//		Frames[ActiveFrameIndex].ApplyPoses();
//
//		if (UpdateRate < 0.f)
//		{
//			UpdateRate = CalcDefaultUpdateRate(8);
//		}
//
//		// Use delay since the first frame was already applied
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, UpdateRate, true, UpdateRate);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Previous replay was already running, restarting.."), *FString(__FUNCTION__), __LINE__);
//		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//		Replay(StartTime, EndTime, UpdateRate, bLoop);
//	}
//}
//
//// Pause / start replay
//void ASLVizEpisodeReplayManager::ToggleReplay()
//{
//	if (ReplayTimerHandle.IsValid())
//	{
//		if (GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
//		}
//		else
//		{
//			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
//	}
//}
//
//// Set replay to pause or play
//void ASLVizEpisodeReplayManager::SetPauseReplay(bool bPause)
//{
//	if (ReplayTimerHandle.IsValid())
//	{
//		if (bPause && GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
//		}
//		else if (!bPause && GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
//	}
//}
//
//// Replay timer callback
//void ASLVizEpisodeReplayManager::TimerCallback()
//{
//	if (ActiveFrameIndex <= ReplayLastFrameIndex - ReplayStepSize)
//	{
//		// Goto next step
//		ActiveFrameIndex += ReplayStepSize;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else if (bLoopTheReplay)
//	{
//		// Goto first frame + the remaining steps
//		ActiveFrameIndex = ReplayLastFrameIndex - ActiveFrameIndex;
//		Frames[ActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply last frame (even if it is not a full step)
//		if (ActiveFrameIndex != ReplayLastFrameIndex)
//		{
//			ActiveFrameIndex = ReplayLastFrameIndex;
//			Frames[ActiveFrameIndex].ApplyPoses();
//		}
//		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
//	}
//}


// Calculate an average update rate from the timestamps
void ASLVizEpisodeReplayManager::CalcDefaultUpdateRate(int32 MaxNumSteps)
{
	if (!EpisodeDataFull.IsValid() || !EpisodeDataCompact.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid, cannot aprox a default update rate"),
			*FString(__FUNCTION__), __LINE__);
		ReplayDefaultUpdateRate = 0.f;
	}

	const int32 NumFrames = EpisodeDataFull.Timestamps.Num();
	if (MaxNumSteps > NumFrames / 2)
	{
		MaxNumSteps = NumFrames / 2;
	}

	// Make an average value using the MaxNumSteps entries from the first quarter
	int32 StartFrameIdx = NumFrames / 4;
	int32 EndFrameIdx = StartFrameIdx + MaxNumSteps;
	float UpdateRate = 0.f;
	if (EndFrameIdx > NumFrames)
	{
		EndFrameIdx = NumFrames;
	}

	// Start from the first quarter, at the beginning one might have some outliers due to loading time spikes
	for (int32 Idx = StartFrameIdx; Idx < EndFrameIdx - 1; ++Idx)
	{
		UpdateRate += (EpisodeDataFull.Timestamps[Idx+1] - EpisodeDataFull.Timestamps[Idx]);
	}

	ReplayDefaultUpdateRate = UpdateRate / ((float)(MaxNumSteps - 1));
}

// Apply frame poses
void ASLVizEpisodeReplayManager::ApplyPoses(const FSLVizEpisodeFrameData& Frame)
{
	for (const auto& ActorPosePair : Frame.ActorPoses)
	{
		ActorPosePair.Key->SetActorTransform(ActorPosePair.Value);
	}

	for (const auto& PMCBonePosesPair : Frame.BonePoses)
	{
		UPoseableMeshComponent* PMC = PMCBonePosesPair.Key;
		for (const auto& BoneIndexPosePair : PMCBonePosesPair.Value)
		{
			PMC->SetBoneTransformByName(PMC->GetBoneName(BoneIndexPosePair.Key), BoneIndexPosePair.Value, EBoneSpaces::WorldSpace);
		}
	}
}
