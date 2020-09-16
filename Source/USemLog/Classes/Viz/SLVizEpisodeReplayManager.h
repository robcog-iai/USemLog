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
	//TArray<TPair<AActor*, FTransform>> ActorPoses;
	TMap<AActor*, FTransform> ActorPoses;

	// Array of the skeletal components and their bone poses (the actor locations are included above)
	//TArray<TPair<UPoseableMeshComponent*, TArray<TPair<int32, FTransform>>>> BonePoses;
	TMap<UPoseableMeshComponent*, TMap<int32, FTransform>> BonePoses;

	// Default ctor
	FSLVizEpisodeFrameData() {};

	// Reserve array size ctor
	FSLVizEpisodeFrameData(int32 ActorArraySize, int32 SkelArraySize)
	{
		ActorPoses.Reserve(ActorArraySize);
		BonePoses.Reserve(SkelArraySize);
	};
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
	// Called when the game starts or when spaewned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Set world as visual only, remove unnecessary non-visual components/actors from world, create poseable skeletal mesh components etc.
	void SetWorldAsVisualOnly();

	// True if initalized
	bool IsWorldSetASVisualOnly() const { return bWorldSetAsVisualOnly; };

	// Load episode data
	void LoadEpisode(const FSLVizEpisodeData& InEpisodeData);

//
//	// Add a frame (make sure these are ordered)
//	void AddFrame(float Timestamp,
//		const TMap<AStaticMeshActor*, FTransform>& EntityPoses,
//		const TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses);
//
//	// Clear all frames related data, keep mappings
//	void ClearFrames();
//
//	// Goto the frame nearest to the timestamp
//	void GoTo(float Timestamp);
//
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
//	// Set replay to pause or play
//	void SetPauseReplay(bool bPause);
//	
//
//protected:
//	// Replay timer callback
//	void TimerCallback();


private:
	// Make sure the mesh of the pawn or spectator is not visible in the world
	void HidePawnOrSpecator();

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	void SetActorsAsVisualsOnly();

	// Add a poseable mesh component clone to the skeletal actors
	void AddPoseablMeshComponentsToSkeletalActors();
	
	// Remove any unnecessary actors or components
	void RemoveUnnecessaryActorsOrComponents();

	// Calculate the default update rate as an average of a given number of frames delta timestamps
	void CalcDefaultUpdateRate(int32 MaxNumSteps);

protected:
	// True if the world is set as visual only
	uint8 bWorldSetAsVisualOnly : 1;

	// True if there is an episode loaded
	uint8 bEpisodeLoaded : 1;

	// True if the replay should loop
	uint8 bReplayShouldLoop : 1;

	// True if it currently in an active replay
	uint8 bReplaying : 1;

	// Episode data containing all the information at every timestamp (used for fast goto calls)
	FSLVizEpisodeData EpisodeDataRaw;

	// Episode data containing only individual changes (used for fast replays)
	FSLVizEpisodeData EpisodeDataCompact;

	// Current frame index
	int32 ReplayActiveFrameIndex;

	// Replay start frame
	int32 ReplayFirstFrameIndex;

	// Replay end frame
	int32 ReplayLastFrameIndex;

	// Replay step size
	int32 ReplayStepSize;

	// Default replay update rate
	float ReplayDefaultUpdateRate;
};


