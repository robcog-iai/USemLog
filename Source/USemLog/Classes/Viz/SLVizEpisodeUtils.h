// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

// Forward declarations
class UWorld;
class AActor;
class ASLIndividualManager;
struct FSLVizEpisodeData;

/**
 * Viz visual parameters (color and material type)
 */
struct USEMLOG_API FSLVizEpisodeUtils
{
public:
	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	static void SetActorsAsVisualsOnly(UWorld* World);

	// Add a poseable mesh component clone to the skeletal actors
	static void AddPoseablMeshComponentsToSkeletalActors(UWorld* World);	

	// Build the full replay episode data from the mongo compact form (returns true if no errors occured)
	static bool BuildEpisodeData(ASLIndividualManager* IndividualManager, 
		const TArray<TPair<float, TMap<FString, FTransform>>>& InMongoEpisodeData,
		FSLVizEpisodeData& OutVizEpisodeData);

	// Executes a binary search for element Item in array Array using the <= operator (from ProfilerCommon::FBinaryFindIndex)
	static int32 BinarySearchLessEqual(const TArray<float>& Array, float Value);

private:
	// Check if actor requires any special attention when switching to visual only world (return true if the components should be left alone)
	static bool IsSpecialCaseActor(AActor* Actor);

	// Remove actor components that are not required in the 'visual only' world (e.g. controllers)
	static void RemoveUnnecessaryComponents(AActor* Actor);
};



