// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizEpisodeManager.h"
#include "Viz/SLVizEpisodeUtils.h"
#include "Components/PoseableMeshComponent.h"

// Sets default values
ASLVizEpisodeManager::ASLVizEpisodeManager()
{
	// Allow ticking, disable it by default (used for episode replay)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bWorldSetAsVisualOnly = false;
	bEpisodeLoaded = false;
	bLoopReplay = false;
	bReplayRunning = false;

	EpisodeDefaultUpdateRate = 0.f;
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
void ASLVizEpisodeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ApplyNextFrameChanges())
	{
		if (bLoopReplay)
		{
			ActiveFrameIndex = ReplayFirstFrameIndex;
			GotoFrame(ActiveFrameIndex);
		}
		else
		{
			StopReplay();
		}
	}
}

// Set the whole world as a visual, disable physics, collisions, attachments, unnecesary components
void ASLVizEpisodeManager::ConvertWorld()
{
	if (bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d World has already been set as visual only.."),
			*FString(__FUNCTION__), __LINE__);
		return;
	}

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	FSLVizEpisodeUtils::SetActorsAsVisualsOnly(GetWorld());

	// Add a poseable mesh component clone to the skeletal actors
	FSLVizEpisodeUtils::AddPoseablMeshComponentsToSkeletalActors(GetWorld());

	// Mark world as visual only
	bWorldSetAsVisualOnly = true;
}

// Load episode data
void ASLVizEpisodeManager::LoadEpisode(const FSLVizEpisodeData& InEpisodeData)
{
	// Check if the data is valid
	if (!InEpisodeData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid to load.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Stop any active replay
	if (bReplayRunning)
	{
		// Stop replay
		StopReplay();
	}

	// Clear any previous episode
	ClearEpisode();

	// Set the episode data
	EpisodeData = InEpisodeData;

	// Calculate a default update rate  
	CalcRealtimeAproxUpdateRateValue(256);

	// Mark the episode loaded flag to true
	bEpisodeLoaded = true;

	// Goto first frame
	GotoFrame(0);
}

// Remove episode data
void ASLVizEpisodeManager::ClearEpisode()
{
	StopReplay();
	EpisodeData.Clear();
	ActiveFrameIndex = INDEX_NONE;
	ReplayFirstFrameIndex = INDEX_NONE;
	ReplayLastFrameIndex = INDEX_NONE;
	bEpisodeLoaded = false;
	bReplayRunning = false;
	SetActorTickEnabled(false);
}

// Set visual world as in the given frame 
bool ASLVizEpisodeManager::GotoFrame(int32 FrameIndex)
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

	if(!EpisodeData.FullFrames.IsValidIndex(FrameIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Frame index is not valid, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	ActiveFrameIndex = FrameIndex;
	ApplyPoses(EpisodeData.FullFrames[FrameIndex]);

	//UE_LOG(LogTemp, Log, TEXT("%s::%d Applied poses from frame %d.."), *FString(__FUNCTION__), __LINE__, ActiveFrameIndex);
	return true;
}

// Set visual world as in the given timestamp (binary search for nearest index)
bool ASLVizEpisodeManager::GotoFrame(float Timestamp)
{
	return GotoFrame(FSLVizEpisodeUtils::BinarySearchLessEqual(EpisodeData.Timestamps, Timestamp));
}

// Play episode with the given parameters
bool ASLVizEpisodeManager::Play(const FSLVizEpisodePlayParams& PlayParams)
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

	// Stop any previous replays
	StopReplay();

	// Set first frame
	ReplayFirstFrameIndex = PlayParams.StartTime < 0 ? 0 
		: FSLVizEpisodeUtils::BinarySearchLessEqual(EpisodeData.Timestamps, PlayParams.StartTime);

	// Set last frame
	ReplayLastFrameIndex = PlayParams.EndTime < 0 ? EpisodeData.Timestamps.Num()
		: PlayParams.EndTime < PlayParams.StartTime ? EpisodeData.Timestamps.Num() 
			: FSLVizEpisodeUtils::BinarySearchLessEqual(EpisodeData.Timestamps, PlayParams.EndTime);

	// Should the replay be looped
	bLoopReplay = PlayParams.bLoop;

	// Goto first frame
	GotoFrame(ReplayFirstFrameIndex);

	// Start playing the frames
	StartReplay();

	return false;
}

// Play whole episode
bool ASLVizEpisodeManager::PlayEpisode()
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

	// Set replay flags
	ReplayFirstFrameIndex = 0;
	ReplayLastFrameIndex = EpisodeData.Timestamps.Num();

	// Goto first frame
	GotoFrame(ReplayFirstFrameIndex);

	// Enable tick with the given update rate
	SetActorTickEnabled(true);
	bReplayRunning = true;

	return true;
}

// Play given frames
bool ASLVizEpisodeManager::PlayFrames(int32 FirstFrame, int32 LastFrame)
{
	if (FirstFrame < 0 || LastFrame < 0 || FirstFrame > LastFrame || LastFrame > EpisodeData.Timestamps.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d FirstFrame=%d and LastFrame=%d are not valid.."), *FString(__FUNCTION__), __LINE__, FirstFrame, LastFrame);
		return false;
	}
	// Set replay flags
	ReplayFirstFrameIndex = FirstFrame;
	ReplayLastFrameIndex = LastFrame;

	// Goto first frame
	GotoFrame(ReplayFirstFrameIndex);

	// Enable tick with the preconfigured update rate	
	SetActorTickEnabled(true);
	bReplayRunning = true;

	return true;
}

// Play the episode timeline
bool ASLVizEpisodeManager::PlayTimeline(float StartTime, float EndTime)
{
	if (StartTime < 0 || EndTime < 0 || StartTime > EndTime)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d StartTime=%f and EndTime=%f are not valid.."), *FString(__FUNCTION__), __LINE__, StartTime, EndTime);
		return false;
	}
	int32 StartFrameIndex = FSLVizEpisodeUtils::BinarySearchLessEqual(EpisodeData.Timestamps, StartTime);
	int32 EndFrameIndex = FSLVizEpisodeUtils::BinarySearchLessEqual(EpisodeData.Timestamps, EndTime);
	return PlayFrames(StartFrameIndex, EndFrameIndex);
}

// Set replay parameters
void ASLVizEpisodeManager::SetReplayParams(bool bLoop, float UpdateRate, int32 StepSize)
{
	bLoopReplay = bLoop;
	UpdateRate > 0.f ? SetActorTickInterval(UpdateRate) : SetActorTickInterval(EpisodeDefaultUpdateRate);
}

// Set replay to pause or play
void ASLVizEpisodeManager::SetPauseReplay(bool bPause)
{
	if (bReplayRunning == bPause)
	{
		SetActorTickEnabled(!bPause);
		bReplayRunning = !bPause;
	}
}

// Stop replay, goto first frame
void ASLVizEpisodeManager::StopReplay()
{
	if (bReplayRunning || IsActorTickEnabled())
	{
		SetActorTickEnabled(false);
		bReplayRunning = false;
		GotoFrame(0);
		ReplayFirstFrameIndex = INDEX_NONE;
		ReplayLastFrameIndex = INDEX_NONE;
	}
}

//// Goto the next nth frame
//void ASLVizEpisodeManager::Next(int32 StepSize, bool bLoop)
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
//void ASLVizEpisodeManager::Previous(int32 StepSize, bool bLoop)
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
//void ASLVizEpisodeManager::Replay(int32 StepSize, float UpdateRate, bool bLoop)
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
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeManager::TimerCallback, 0.01, true, 0.01);
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
//void ASLVizEpisodeManager::Replay(float StartTime, float EndTime, int32 StepSize, float UpdateRate, bool bLoop)
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
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeManager::TimerCallback, UpdateRate, true, UpdateRate);
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
//void ASLVizEpisodeManager::TimerCallback()
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

// Apply the next frames changes
bool ASLVizEpisodeManager::ApplyNextFrameChanges()
{
	if (ActiveFrameIndex < ReplayLastFrameIndex)
	{
		ActiveFrameIndex++;
		//if (EpisodeData.CompactFrames.IsValidIndex(ActiveFrameIndex))
		if (EpisodeData.FullFrames.IsValidIndex(ActiveFrameIndex))
		{
			//ApplyPoses(EpisodeData.CompactFrames[ActiveFrameIndex]);
			ApplyPoses(EpisodeData.FullFrames[ActiveFrameIndex]);
			return true;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveFrameIndex=%d (Num=%d) is not valid, this should not happen.."),
			//	*FString(__FUNCTION__), __LINE__, ActiveFrameIndex, EpisodeData.CompactFrames.Num());
			UE_LOG(LogTemp, Warning, TEXT("%s::%d ActiveFrameIndex=%d (Num=%d) is not valid, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, ActiveFrameIndex, EpisodeData.FullFrames.Num());
			ActiveFrameIndex--;
		}
	}
	return false;	
}

// Start replay
void ASLVizEpisodeManager::StartReplay()
{
	// Enable tick with the given update rate
	SetActorTickEnabled(true);
	bReplayRunning = true;}

// Apply frame poses
void ASLVizEpisodeManager::ApplyPoses(const FSLVizEpisodeFrameData& Frame)
{
	for (const auto& ActorPosePair : Frame.ActorPoses)
	{
		// todo, static components can be ignored (might make sense to remove them form the episode data)
		if (ActorPosePair.Key->GetRootComponent()->Mobility != EComponentMobility::Static)
		{
			ActorPosePair.Key->SetActorTransform(ActorPosePair.Value);
		}
	}

	// todo, without this multiple iteration the bones are weirdly offseted
	for (int32 Idx = 0; Idx < 5; Idx++)
	{
		for (const auto& PMCBonePosesPair : Frame.BonePoses)
		{
			UPoseableMeshComponent* PMC = PMCBonePosesPair.Key;
			for (const auto& BoneIndexPosePair : PMCBonePosesPair.Value)
			{
				const FName BoneName = PMC->GetBoneName(BoneIndexPosePair.Key);
				PMC->SetBoneTransformByName(BoneName, BoneIndexPosePair.Value, EBoneSpaces::WorldSpace);
			}
		}
	}
}

// Calculate an approximation of the update rate value to coincide with realtime
void ASLVizEpisodeManager::CalcRealtimeAproxUpdateRateValue(int32 MaxNumSteps)
{
	if (!EpisodeData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid, cannot aprox a default update rate"),
			*FString(__FUNCTION__), __LINE__);
		EpisodeDefaultUpdateRate = 0.f;
	}

	const int32 NumFrames = EpisodeData.Timestamps.Num();
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
		UpdateRate += (EpisodeData.Timestamps[Idx + 1] - EpisodeData.Timestamps[Idx]);
	}

	EpisodeDefaultUpdateRate = UpdateRate / ((float)(MaxNumSteps - 1));

	UE_LOG(LogTemp, Log, TEXT("%s::%d Default update rate set to %f seconds .."),
		*FString(__FUNCTION__), __LINE__, EpisodeDefaultUpdateRate);
}

