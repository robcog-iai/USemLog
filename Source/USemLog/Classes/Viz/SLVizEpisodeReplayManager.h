// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizEpisodeReplayManager.generated.h"

// Forward declaration
class UPoseableMeshComponent;

/*
* Holds the poses of all the individuals in the world
*/
struct FSLVizEpisodeFrameData
{
	// Array of the actors and their poses
	TMap<AActor*, FTransform> ActorPoses;

	// Array of the skeletal components and their bone poses (the actor locations are included above)
	TMap<UPoseableMeshComponent*, TMap<int32, FTransform>> BonePoses;
};

/*
* Holds the frames from the recorded episode
*/
struct FSLVizEpisodeData
{
	// Array of the timestamps
	TArray<float> Timestamps;

	// Array of the frames
	TArray<FSLVizEpisodeFrameData> Frames;

	// Default ctor
	FSLVizEpisodeData() {};

	// Reserve array size ctor
	FSLVizEpisodeData(int32 ArraySize)
	{
		Timestamps.Reserve(ArraySize);
		Frames.Reserve(ArraySize);
	};

	// Check if there is data in the episode and it is in sync
	bool IsValid() const { return Timestamps.Num() > 2 && Timestamps.Num() == Frames.Num(); };

	// Clear all the data in the episode
	void Clear() { Timestamps.Empty(); Frames.Empty(); };
};

/**
 * Class to load and skim through episodes
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Episode Replay Manager")
class USEMLOG_API ASLVizEpisodeReplayManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizEpisodeReplayManager();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Set world as visual only, remove unnecessary non-visual components/actors from world, create poseable skeletal mesh components etc.
	void SetWorldAsVisualOnly();

	// True if initalized
	bool IsWorldSetASVisualOnly() const { return bWorldSetAsVisualOnly; };

	// Load episode data
	void LoadEpisode(const FSLVizEpisodeData& InEpisodeDataFull, const FSLVizEpisodeData& InEpisodeDataCompact);

	// Check if an episode is loaded
	bool IsEpisodeLoaded() const { return bEpisodeLoaded; };

	// Remove episode data
	void ClearEpisode();

	// Set visual world as in the given frame 
	bool GotoFrame(int32 FrameIndex);

	// Set visual world as in the given timestamp (binary search for nearest index)
	bool GotoFrame(float Timestamp);

	// Play whole episode
	bool Play(bool bLoop = false, float UpdateRate = -1.f, int32 StepSize = 1);

	// Play given frames
	bool PlayFrames(int32 FirstFrame, int32 LastFrame, bool bLoop = false, float UpdateRate = -1.f, int32 StepSize = 1);

	// Play the episode timeline
	bool PlayTimeline(float StartTime, float EndTime, bool bLoop = false, float UpdateRate = -1.f, int32 StepSize = 1);

	// Set replay to pause or play
	void SetPauseReplay(bool bPause);

	// Stop replay, goto first frame
	void StopReplay();

//	// Goto the next nth frame
//	void Next(int32 StepSize = 1, bool bLoop = false);
//
//	// Goto the previous nth frame
//	void Previous(int32 StepSize = 1, bool bLoop = false);
//
//	// Replay all the episode with the given update rate (by default the update rate is calculated as the average of the first X frames)
//	void Replay(int32 StepSize = 1, float UpdateRate = -1.f, bool bLoop = true);
//
//	// Replay the episode between the timestamp with the given update rate (by default the update rate is calculated as the average of the first X frames)
//	void Replay(float StartTime, float EndTime, int32 StepSize = 1, float UpdateRate = -1.f, bool bLoop = true);
//
//	// Pause / start replay
//	void ToggleReplay();
//
//protected:
//	// Replay timer callback
//	void TimerCallback();


private:
	// Apply the compact episode data given frame changes
	void ApplyCompactFrameChanges(int32 FrameIndex);

	// Calculate an approximation of the update rate value to coincide with realtime
	void CalcRealtimeAproxUpdateRateValue(int32 MaxNumSteps);

	// Apply frame poses
	void ApplyPoses(const FSLVizEpisodeFrameData& Frame);

protected:
	// True if the world is set as visual only
	uint8 bWorldSetAsVisualOnly : 1;

	// True if there is an episode loaded
	uint8 bEpisodeLoaded : 1;

	// True if the replay should loop
	uint8 bLoopReplay : 1;

	// True if it currently in an active replay
	uint8 bReplayRunning : 1;

	// Episode data containing all the information at every timestamp (used for fast goto calls)
	FSLVizEpisodeData EpisodeDataFull;

	// Episode data containing only individual changes (used for fast replays)
	FSLVizEpisodeData EpisodeDataCompact;

	// Current frame index
	int32 ActiveFrameIndex;

	// Replay start frame
	int32 ReplayFirstFrameIndex;

	// Replay end frame
	int32 ReplayLastFrameIndex;

	// Replay step size
	int32 ReplayStepSize;

	// Default replay update rate
	float ReplayAproxRealtimeUpdateRateValue;
};


