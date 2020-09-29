// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizEpisodeReplayManager.h"
#include "Viz/SLVizEpisodeReplayUtils.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ASLVizEpisodeReplayManager::ASLVizEpisodeReplayManager()
{
	// Allow ticking, disable it by default (used for episode replay)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bWorldSetAsVisualOnly = false;
	bEpisodeLoaded = false;
	bLoopReplay = false;
	bReplayRunning = false;

	ReplayViewTarget = nullptr;

	ReplayAproxRealtimeUpdateRateValue = 0.f;
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

	if (ActiveFrameIndex < ReplayLastFrameIndex)
	{		
		ApplyCompactFrameChanges(ActiveFrameIndex + 1);
	}
	else
	{
		if (bLoopReplay)
		{
			ActiveFrameIndex = ReplayFirstFrameIndex;
			GotoFrame(ActiveFrameIndex);
		}
		else
		{
			SetActorTickEnabled(false);
			bReplayRunning = false;
		}
	}
}

// Set the whole world as a visual, disable physics, collisions, attachments, unnecesary components
void ASLVizEpisodeReplayManager::SetWorldAsVisualOnly()
{
	if (bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World has already been set as visual only.."),
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
	if (bReplayRunning)
	{
		// Stop replay
	}

	// Clear any previous episode
	ClearEpisode();

	// Set the episode data
	EpisodeDataFull = InEpisodeDataFull;
	EpisodeDataCompact = InEpisodeDataCompact;

	// Calculate a default update rate  
	CalcRealtimeAproxUpdateRateValue(256);

	// Mark the episode loaded flag to true
	bEpisodeLoaded = true;

	// Goto first frame
	GotoFrame(0);
}

// Remove episode data
void ASLVizEpisodeReplayManager::ClearEpisode()
{
	EpisodeDataFull.Clear();
	EpisodeDataCompact.Clear();
	ReplayViewTarget = nullptr;
	ActiveFrameIndex = INDEX_NONE;
	ReplayFirstFrameIndex = INDEX_NONE;
	ReplayLastFrameIndex = INDEX_NONE;
	bEpisodeLoaded = false;
	bReplayRunning = false;
	SetActorTickEnabled(false);
}

// Set visual world as in the given frame 
bool ASLVizEpisodeReplayManager::GotoFrame(int32 FrameIndex, AActor* ViewTarget)
{
	if (!bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World is not set as visual only.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	if (!bEpisodeLoaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No episode is loaded.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	if(!EpisodeDataFull.Frames.IsValidIndex(FrameIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Frame index is not valid, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	ApplyPoses(EpisodeDataFull.Frames[FrameIndex]);
	ActiveFrameIndex = FrameIndex;

	//UE_LOG(LogTemp, Log, TEXT("%s::%d Applied poses from frame %d.."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
	return true;
}

// Set visual world as in the given timestamp (binary search for nearest index)
bool ASLVizEpisodeReplayManager::GotoFrame(float Timestamp, AActor* ViewTarget)
{
	return GotoFrame(FSLVizEpisodeReplayUtils::BinarySearchLessEqual(EpisodeDataFull.Timestamps, Timestamp), ViewTarget);
}

// Play whole episode
bool ASLVizEpisodeReplayManager::PlayEpisode(AActor* ViewTarget)
{
	if (!bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World is not set as visual only.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	if (!bEpisodeLoaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No episode is loaded.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	ReplayViewTarget = ViewTarget;

	// Set replay flags
	ReplayFirstFrameIndex = 0;
	ReplayLastFrameIndex = EpisodeDataFull.Timestamps.Num();

	// Goto first frame
	GotoFrame(ReplayFirstFrameIndex, ViewTarget);

	// Enable tick with the given update rate
	SetActorTickEnabled(true);

	bReplayRunning = true;
	return true;
}

// Play given frames
bool ASLVizEpisodeReplayManager::PlayFrames(int32 FirstFrame, int32 LastFrame, AActor* ViewTarget)
{
	if (FirstFrame < 0 || LastFrame < 0 || FirstFrame > LastFrame || LastFrame > EpisodeDataFull.Timestamps.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d FirstFrame=%d and LastFrame=%d are not valid.."), *FString(__FUNCTION__), __LINE__, FirstFrame, LastFrame);
		return false;
	}

	ReplayViewTarget = ViewTarget;

	// Set replay flags
	ReplayFirstFrameIndex = FirstFrame;
	ReplayLastFrameIndex = LastFrame;

	// Goto first frame
	GotoFrame(ReplayFirstFrameIndex, ViewTarget);

	// Enable tick with the preconfigured update rate	
	SetActorTickEnabled(true);

	bReplayRunning = true;
	return true;
}

// Play the episode timeline
bool ASLVizEpisodeReplayManager::PlayTimeline(float StartTime, float EndTime, AActor* ViewTarget)
{
	if (StartTime < 0 || EndTime < 0 || StartTime > EndTime)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d StartTime=%f and EndTime=%f are not valid.."), *FString(__FUNCTION__), __LINE__, StartTime, EndTime);
		return false;
	}
	int32 StartFrameIndex = FSLVizEpisodeReplayUtils::BinarySearchLessEqual(EpisodeDataFull.Timestamps, StartTime);
	int32 EndFrameIndex = FSLVizEpisodeReplayUtils::BinarySearchLessEqual(EpisodeDataFull.Timestamps, EndTime);
	return PlayFrames(StartFrameIndex, EndFrameIndex, ViewTarget);
}

// Set replay parameters
void ASLVizEpisodeReplayManager::SetReplayParams(bool bLoop, float UpdateRate, int32 StepSize)
{
	bLoopReplay = bLoop;
	UpdateRate > 0.f ? SetActorTickInterval(UpdateRate) : SetActorTickInterval(ReplayAproxRealtimeUpdateRateValue);
}

// Set replay to pause or play
void ASLVizEpisodeReplayManager::SetPauseReplay(bool bPause)
{
	if (bReplayRunning == bPause)
	{
		SetActorTickEnabled(!bPause);
		bReplayRunning = !bPause;
	}
}

// Stop replay, goto first frame
void ASLVizEpisodeReplayManager::StopReplay()
{
	if (bReplayRunning)
	{
		SetActorTickEnabled(false);
		bReplayRunning = false;
		GotoFrame(0);
		ReplayViewTarget = nullptr;
		ReplayFirstFrameIndex = INDEX_NONE;
		ReplayLastFrameIndex = INDEX_NONE;
	}
}

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
//			UpdateRate = CalcRealtimeAproxUpdateRateValue(8);
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
//			UpdateRate = CalcRealtimeAproxUpdateRateValue(8);
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


// Apply the compact episode data given frame changes
void ASLVizEpisodeReplayManager::ApplyCompactFrameChanges(int32 FrameIndex)
{
	if (!EpisodeDataCompact.Frames.IsValidIndex(FrameIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Frame index is not valid, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
	ApplyPoses(EpisodeDataCompact.Frames[FrameIndex]);
	ActiveFrameIndex = FrameIndex;
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

	if (ReplayViewTarget)
	{
		if (FPC)
		{
			FPC->SetViewTarget(ReplayViewTarget);
		}
		else
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->SetViewTarget(ReplayViewTarget);
				FPC = PC;
			}
		}
	}
}

// Calculate an approximation of the update rate value to coincide with realtime
void ASLVizEpisodeReplayManager::CalcRealtimeAproxUpdateRateValue(int32 MaxNumSteps)
{
	if (!EpisodeDataFull.IsValid() || !EpisodeDataCompact.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid, cannot aprox a default update rate"),
			*FString(__FUNCTION__), __LINE__);
		ReplayAproxRealtimeUpdateRateValue = 0.f;
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
		UpdateRate += (EpisodeDataFull.Timestamps[Idx + 1] - EpisodeDataFull.Timestamps[Idx]);
	}

	ReplayAproxRealtimeUpdateRateValue = UpdateRate / ((float)(MaxNumSteps - 1));

	UE_LOG(LogTemp, Log, TEXT("%s::%d Default update rate set to %f seconds .."),
		*FString(__FUNCTION__), __LINE__, ReplayAproxRealtimeUpdateRateValue);
}

