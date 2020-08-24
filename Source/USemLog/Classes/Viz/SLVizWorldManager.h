// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/PoseableMeshComponent.h"
#include "TimerManager.h"
#include "SLVizWorldManager.generated.h"

/*
* Structure holding the world state at a given timestamp
*/
struct FSLVizWorldStateFrame
{
	// The pose of all the entities (id to pose)
	TMap<AStaticMeshActor*, FTransform> EntityPoses;

	// The poses of all the skeletal actors and their bones (the poseable mesh component
	TMap<UPoseableMeshComponent*, TPair<FTransform, TMap<FString, FTransform>>> SkeletalPoses;

	// Apply poses of this frame
	void ApplyPoses()
	{
		for (const auto& EP : EntityPoses)
		{
			EP.Key->SetActorTransform(EP.Value);
		}

		for (auto& SkP : SkeletalPoses)
		{
			SkP.Key->GetOwner()->SetActorTransform(SkP.Value.Key);

			for (auto& BP : SkP.Value.Value)
			{
				SkP.Key->SetBoneTransformByName(*BP.Key, BP.Value, EBoneSpaces::WorldSpace);
			}
		}
	}
};


/**
 * Class to load and skim through episodes
 */
UCLASS()
class USEMLOG_API ASLVizWorldManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLVizWorldManager();

	// Set world to visual only, create poseable skeletal mesh components
	void Setup();

	// True if initalized
	bool IsReady() const { return bIsReady; };

	// Add a frame (make sure these are ordered)
	void AddFrame(float Timestamp,
		const TMap<AStaticMeshActor*, FTransform>& EntityPoses,
		const TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses);

	// Clear all frames related data, keep mappings
	void ClearFrames();

	// Goto the frame nearest to the timestamp
	void GoTo(float Timestamp);

	// Goto the next nth frame
	void Next(int32 StepSize = 1, bool bLoop = false);

	// Goto the previous nth frame
	void Previous(int32 StepSize = 1, bool bLoop = false);

	// Replay all the episode with the given update rate (by default the update rate is calculated as the average of the first X frames)
	void Replay(int32 StepSize = 1, float UpdateRate = -1.f, bool bLoop = true);

	// Replay the episode between the timestamp with the given update rate (by default the update rate is calculated as the average of the first X frames)
	void Replay(float StartTime, float EndTime, int32 StepSize = 1, float UpdateRate = -1.f, bool bLoop = true);

	// Pause / start replay
	void ToggleReplay();

	// Set replay to pause or play
	void SetPauseReplay(bool bPause);

protected:
	// Replay timer callback
	void TimerCallback();

private:
	// Remove unnecesary non-visual components/actors from world
	void SetupWorldAsVisual();

	// Check if actor or any of its components should be removed
	void CleanActor(AActor* Actor) const;

	// Hide skeletal mesh components and create new visible poseable mesh components
	UPoseableMeshComponent* CreateNewPoseableMeshComponent(ASkeletalMeshActor* SkeletalActor) const;

	// Calculate an average update rate from the timestamps
	float GetReplayUpdateRateFromTimestampsAverage(int32 Steps = 10) const;

protected:
	// Init flag
	bool bIsReady;

	// Episode to skim through, synched with the timestamp array
	TArray<FSLVizWorldStateFrame> Frames;

	// Pre sorted array of the timestamp synched with the frames array
	TArray<float> Timestamps;

	// Current frame index
	int32 CurrFrameIdx;

	// Replay start frame
	int32 ReplayFirstFrameIdx;

	// Replay end frame
	int32 ReplayLastFrameIdx;

	// Replay step size
	int32 ReplayStepSize;

	// True if the replay should loop
	bool bReplayLoop;

	// Timer handle for the replays
	FTimerHandle ReplayTimerHandle;

private:
	// Map of the skeletal mesh actor to its poseable mesh component
	TMap<ASkeletalMeshActor*, UPoseableMeshComponent*> SkeletalActorToPoseableMeshMap;

};


