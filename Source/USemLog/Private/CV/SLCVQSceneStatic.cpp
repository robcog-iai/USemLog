// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVQSceneStatic.h"
#include "Individuals/SLIndividualManager.h"
#include "GameFramework/Actor.h"

// Virtual implementation for the scene initialization
bool USLCVQSceneStatic::InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Iterate the scene actors, cache their original world position,
	for (const auto& Id : Ids)
	{
		if (auto CurrActor = IndividualManager->GetIndividualActor(Id))
		{
			// Cache the original world pose
			FTransform WorldPose = CurrActor->GetTransform();
			SceneActorPoses.Add(CurrActor, WorldPose);
		}
	}

	// Calculate the sphere bounds radius and the centroid/barycenter of the scene
	FVector SceneCentroidLocation;
	for (const auto& ActPosePair : SceneActorPoses)
	{
		FTransform WorldPose = ActPosePair.Value;
		SceneCentroidLocation += WorldPose.GetLocation();
	}
	SceneCentroidLocation /= SceneActorPoses.Num();

	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(-SceneCentroidLocation);
	}

	return SceneActorPoses.Num() > 0;
}


