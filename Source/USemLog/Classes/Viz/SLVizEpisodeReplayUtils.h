// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

// Forward declarations
class UWorld;
class AActor;

/**
 * Viz visual parameters (color and material type)
 */
struct USEMLOG_API FSLVizEpisodeReplayUtils
{
public:
	// Make sure the mesh of the pawn or spectator is not visible in the world
	static void HidePawnOrSpectator(UWorld* World);

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	static void SetActorsAsVisualsOnly(UWorld* World);

	// Add a poseable mesh component clone to the skeletal actors
	static void AddPoseablMeshComponentsToSkeletalActors(UWorld* World);
	
	// Executes a binary search for element Item in array Array using the <= operator (from ProfilerCommon::FBinaryFindIndex)
	static int32 BinarySearchLessEqual(const TArray<float>& Array, float Value);

private:
	// Remove actor components that are not required in the 'visual only' world (e.g. controllers)
	static void RemoveUnnecessaryComponents(AActor* Actor);
};



