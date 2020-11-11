// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SLVizStructs.h"
#include "SLVizEpisodeManager.generated.h"

// Forward declaration
class UPoseableMeshComponent;
class APlayerController;

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
	// Id of the episode
	FString Id;

	// Array of the timestamps
	TArray<float> Timestamps;

	// Array of the compact frames (used for fast gotos)
	TArray<FSLVizEpisodeFrameData> FullFrames;

	// Array of the compact frames (used for fast replays)
	TArray<FSLVizEpisodeFrameData> CompactFrames;

	// Default ctor
	FSLVizEpisodeData() {};

	// Reserve array size ctor
	FSLVizEpisodeData(int32 ArraySize)
	{
		Timestamps.Reserve(ArraySize);
		FullFrames.Reserve(ArraySize);
		CompactFrames.Reserve(ArraySize);
	};

	// Check if there is data in the episode and it is in sync
	bool IsValid() const 
	{
		return Timestamps.Num() > 2 && Timestamps.Num() == FullFrames.Num();
	};

	// Clear all the data in the episode
	void Clear() 
	{
		Id = "";
		Timestamps.Empty(); 
		FullFrames.Empty();
		CompactFrames.Empty();
	};
};


/**
 * Class to load and skim through episodes
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Viz Episode Replay Manager")
class USEMLOG_API ASLVizEpisodeManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizEpisodeManager();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Set world as visual only, remove unnecessary non-visual components/actors from world, create poseable skeletal mesh components etc.
	void ConvertWorld();

	// True if initalized
	bool IsWorldConverted() const { return bWorldSetAsVisualOnly; };

	// Load episode data
	void LoadEpisode(const FSLVizEpisodeData& InEpisodeData);

	// Check if an episode is loaded
	bool IsEpisodeLoaded() const { return bEpisodeLoaded; };

	// Get loaded episode id
	FString GetEpisodeId() const { return EpisodeData.Id; };

	// Remove episode data
	void ClearEpisode();

	// Set visual world as in the given frame 
	bool GotoFrame(int32 FrameIndex);

	// Set visual world as in the given timestamp (binary search for nearest index)
	bool GotoFrame(float Timestamp);

	// Play episode
	bool Play(const FSLVizEpisodePlayParams& PlayParams = FSLVizEpisodePlayParams());

	// Play whole episode
	bool PlayEpisode();

	// Play given frames
	bool PlayFrames(int32 FirstFrame, int32 LastFrame);

	// Play the episode timeline
	bool PlayTimeline(float StartTime, float EndTime);

	// Set replay parameters (loop replay, frame update rate, number of steps per frame)
	void SetReplayParams(bool bLoop, float UpdateRate = -1.f, int32 StepSize = 1);

	// Set replay to pause or play
	void SetPauseReplay(bool bPause);

	// Stop replay, goto first frame
	void StopReplay();

private:
	// Start replay
	void StartReplay();

	// Apply frame poses
	void ApplyPoses(const FSLVizEpisodeFrameData& Frame);

	// Apply next frame changes (return false if there are no more frames)
	bool ApplyNextFrameChanges();

	// Calculate an approximation of the update rate value to coincide with realtime
	void CalcRealtimeAproxUpdateRateValue(int32 MaxNumSteps);

protected:
	// True if the world is set as visual only
	uint8 bWorldSetAsVisualOnly : 1;

	// True if there is an episode loaded
	uint8 bEpisodeLoaded : 1;

	// True if the replay should loop
	uint8 bLoopReplay : 1;

	// True if it currently in an active replay
	uint8 bReplayRunning : 1;

	// Episode data
	FSLVizEpisodeData EpisodeData;

	// Current frame index
	int32 ActiveFrameIndex;

	// Replay start frame
	int32 ReplayFirstFrameIndex;

	// Replay end frame
	int32 ReplayLastFrameIndex;

	// Replay step size
	int32 ReplayStepSize;

	// Default replay update rate
	float EpisodeDefaultUpdateRate;
};


