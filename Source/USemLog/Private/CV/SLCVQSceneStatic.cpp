// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVQSceneStatic.h"
#include "Individuals/SLIndividualManager.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"

// Virtual implementation for the scene initialization
bool USLCVQSceneStatic::InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Iterate the scene actors, cache their original world position,
	for (const auto& Id : Ids)
	{
		if (auto CurrActor = IndividualManager->GetIndividualActor(Id))
		{
			if (auto* AsSMA = Cast<AStaticMeshActor>(CurrActor))
			{
				// Cache the original world pose (no episodic memory is used)
				FTransform WorldPose = AsSMA->GetTransform();
				SceneActorPoses.Add(AsSMA, WorldPose);
			}
			else if (auto* AsSkelMA = Cast<ASkeletalMeshActor>(CurrActor))
			{
				// todo
			}
		}
	}

	//// Calculate the sphere bounds radius and the centroid/barycenter of the scene
	//FVector SceneCentroidLocation;
	//for (const auto& ActPosePair : SceneActorPoses)
	//{
	//	FTransform WorldPose = ActPosePair.Value;
	//	SceneCentroidLocation += WorldPose.GetLocation();
	//}
	//SceneCentroidLocation /= SceneActorPoses.Num();


	// Calculate centroid location
	FVector SceneCentroidLocation;
	FBoxSphereBounds SphereBounds(EForceInit::ForceInit);
	// Add up static mesh bounds
	for (const auto& SMAPosePair : SceneActorPoses)
	{
		// Get the mesh bounds
		FBoxSphereBounds SMBounds = SMAPosePair.Key->GetStaticMeshComponent()->Bounds;

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + SMBounds;
		}
		else
		{
			// First value
			SphereBounds = SMBounds;
		}
	}
	SceneCentroidLocation = SphereBounds.Origin;

	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(-SceneCentroidLocation);
	}

	return SceneActorPoses.Num() > 0;
}


