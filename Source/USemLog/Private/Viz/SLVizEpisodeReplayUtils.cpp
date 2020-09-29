// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizEpisodeReplayUtils.h"
#include "Viz/SLVizEpisodeReplayManager.h"

#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLIndividualTypes.h"

#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "EngineUtils.h"

// IsA's
//#include "Components/LightComponentBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/MovementComponent.h"
//#include "Landscape.h"


// Set actors as visuals only (disable physics, set as movable, clear attachments)
void FSLVizEpisodeReplayUtils::SetActorsAsVisualsOnly(UWorld* World)
{
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		// Make sure all actors have no physics, have no collisions and are movable
		ActItr->DisableComponentsSimulatePhysics();
		ActItr->SetActorEnableCollision(ECollisionEnabled::NoCollision);
		if (ActItr->GetRootComponent())
		{
			ActItr->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}

		// Clear any attachments between actors
		ActItr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Check if there are any components that need to be removed
		if (!IsSpecialCaseActor(*ActItr))
		{
			RemoveUnnecessaryComponents(*ActItr);
		}
	}
}

// Add a poseable mesh component clone to the skeletal actors
void FSLVizEpisodeReplayUtils::AddPoseablMeshComponentsToSkeletalActors(UWorld* World)
{
	for (TActorIterator<ASkeletalMeshActor> SkelActItr(World); SkelActItr; ++SkelActItr)
	{
		UPoseableMeshComponent* PMC = NewObject<UPoseableMeshComponent>(*SkelActItr);
		PMC->SetSkeletalMesh((*SkelActItr)->GetSkeletalMeshComponent()->SkeletalMesh);
		PMC->AttachToComponent((*SkelActItr)->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		PMC->RegisterComponent();
		PMC->bHasMotionBlurVelocityMeshes = false;
		PMC->bPerBoneMotionBlur = false;

		(*SkelActItr)->AddInstanceComponent(PMC); // Makes it appear in the editor
		(*SkelActItr)->AddOwnedComponent(PMC);
		(*SkelActItr)->GetSkeletalMeshComponent()->SetHiddenInGame(true); // Hide original skeletal component
	}
}

// Build the full replay episode data from the mongo compact form
bool FSLVizEpisodeReplayUtils::BuildEpisodeData(ASLIndividualManager* IndividualManager, const TArray<TPair<float, TMap<FString, FTransform>>>& InCompactEpisodeData,
	FSLVizEpisodeData& OutFullEpisodeData, FSLVizEpisodeData& OutCompactEpisodeData)
{
	double ExecBegin = FPlatformTime::Seconds();
	/* First frame */
	// Process first frame (contains all individuals -- the rest of the frames contain only individuals that have moved)
	FSLVizEpisodeFrameData FullFrameData;

	// Iterate individuals with their poses
	for (const auto& IndividualPosePair : InCompactEpisodeData[0].Value)
	{
		const FString IndividualId = IndividualPosePair.Key;
		const FTransform IndividualPose = IndividualPosePair.Value;

		if (auto Individual = IndividualManager->GetIndividual(IndividualId))
		{
			if (Individual->IsA(USLRigidIndividual::StaticClass())
			|| Individual->IsA(USLSkeletalIndividual::StaticClass())
			|| Individual->IsA(USLVirtualViewIndividual::StaticClass()))
			{
				FullFrameData.ActorPoses.Emplace(Individual->GetParentActor(), IndividualPose);
			}
			else if (auto BI = Cast<USLBoneIndividual>(Individual))
			{
				FullFrameData.BonePoses.FindOrAdd(BI->GetPoseableMeshComponent()).Add(BI->GetBoneIndex(), IndividualPose);
			}
			else if (auto VBI = Cast<USLVirtualBoneIndividual>(Individual))
			{
				FullFrameData.BonePoses.FindOrAdd(VBI->GetPoseableMeshComponent()).Add(VBI->GetBoneIndex(), IndividualPose);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not find individual with id=%s, this should not happen, aborting.."),
				*FString(__FUNCTION__), __LINE__, *IndividualPosePair.Key);
			return false;
		}
	}
	// Add the timestamp
	//OutFullEpisodeData.Timestamps[0] = InCompactEpisodeData[0].Key;
	OutFullEpisodeData.Timestamps.Emplace(InCompactEpisodeData[0].Key);
	OutCompactEpisodeData.Timestamps.Emplace(InCompactEpisodeData[0].Key);

	double FirstFrameDuration = FPlatformTime::Seconds() - ExecBegin;

	// Add the individuals poses
	//OutFullEpisodeData.Frames[0] = FullFrameData;
	OutFullEpisodeData.Frames.Emplace(FullFrameData);
	OutCompactEpisodeData.Frames.Emplace(FullFrameData);


	/* Following frames */
	// Process the following frames
	for (int32 FrameIndex = 1; FrameIndex < InCompactEpisodeData.Num(); ++FrameIndex)
	{
		// TODO will emplace make a copy, or one needs to make one here ?
		//FSLVizEpisodeFrameData PrevFrameCopy = FullFrameData;
		FSLVizEpisodeFrameData CompactFrameData;

		// Iterate individuals with their poses
		for (const auto& IndividualPosePair : InCompactEpisodeData[FrameIndex].Value)
		{
			const FString IndividualId = IndividualPosePair.Key;
			const FTransform IndividualPose = IndividualPosePair.Value;
			if (auto Individual = IndividualManager->GetIndividual(IndividualId))
			{
				if (auto RI = Cast<USLRigidIndividual>(Individual))
				{
					// Modify the value in the full/first frame
					//if (auto FoundActorPose = FullFrameData.ActorPoses.Find(RI->GetParentActor())){(*FoundActorPose) = IndividualPose;}
					FullFrameData.ActorPoses.FindChecked(Individual->GetParentActor()) = IndividualPose;
					// Add as new value to the compact frame
					CompactFrameData.ActorPoses.Emplace(Individual->GetParentActor(), IndividualPose);
				}
				else if (auto SkI = Cast<USLSkeletalIndividual>(Individual))
				{
					// Modify the value in the full/first frame
					//if (auto FoundActorPose = FullFrameData.ActorPoses.Find(RI->GetParentActor())){(*FoundActorPose) = IndividualPose;}
					FullFrameData.ActorPoses.FindChecked(Individual->GetParentActor()) = IndividualPose;
					// Add as new value to the compact frame
					CompactFrameData.ActorPoses.Emplace(Individual->GetParentActor(), IndividualPose);
				}
				else if (auto BI = Cast<USLBoneIndividual>(Individual))
				{
					FullFrameData.BonePoses.FindOrAdd(BI->GetPoseableMeshComponent()).Add(BI->GetBoneIndex(), IndividualPose);
					CompactFrameData.BonePoses.FindOrAdd(BI->GetPoseableMeshComponent()).Add(BI->GetBoneIndex(), IndividualPose);
				}
				else if (auto VBI = Cast<USLVirtualBoneIndividual>(Individual))
				{
					FullFrameData.BonePoses.FindOrAdd(VBI->GetPoseableMeshComponent()).Add(VBI->GetBoneIndex(), IndividualPose);
					CompactFrameData.BonePoses.FindOrAdd(VBI->GetPoseableMeshComponent()).Add(VBI->GetBoneIndex(), IndividualPose);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not find individual with id=%s, this should not happen, aborting.."),
					*FString(__FUNCTION__), __LINE__, *IndividualId);
				return false;
			}
		}

		// Add the timestamp
		//OutFullEpisodeData.Timestamps[FrameIndex] = InCompactEpisodeData[FrameIndex].Key;
		OutFullEpisodeData.Timestamps.Emplace(InCompactEpisodeData[FrameIndex].Key);
		OutCompactEpisodeData.Timestamps.Emplace(InCompactEpisodeData[FrameIndex].Key);

		// Add the individuals poses
		//OutFullEpisodeData.Frames[FrameIndex] = FullFrameData;
		OutFullEpisodeData.Frames.Emplace(FullFrameData);
		OutCompactEpisodeData.Frames.Emplace(CompactFrameData);
	}

	double FollowingFramesDuration = FPlatformTime::Seconds() - ExecBegin - FirstFrameDuration;

	UE_LOG(LogTemp, Log, TEXT("%s::%d Durations: first frame=[%f], following frames(num=%d)=[%f], total=[%f] seconds..;"),
		*FString(__func__), __LINE__, FirstFrameDuration, OutFullEpisodeData.Timestamps.Num(),
		FollowingFramesDuration, FPlatformTime::Seconds() - ExecBegin);
	return true;
}


// Executes a binary search for element Item in array Array using the <= operator (from ProfilerCommon::FBinaryFindIndex)
int32 FSLVizEpisodeReplayUtils::BinarySearchLessEqual(const TArray<float>& Array, float Value)
{
	int32 Length = Array.Num();
	int32 Middle = Length;
	int32 Offset = 0;

	while (Middle > 0)
	{
		Middle = Length / 2;
		if (Array[Offset + Middle] <= Value)
		{
			Offset += Middle;
		}
		Length -= Middle;
	}
	return Offset;
}


// Check if actor requires any special attention when switching to visual only world (return true if the components should be left alone)
bool FSLVizEpisodeReplayUtils::IsSpecialCaseActor(AActor* Actor)
{
	//if (Actor->IsA(ALandscape::StaticClass()))
	//{
	//	return true;
	//}
	//else if(Actor->IsA(APlayerController::StaticClass()))
	//{
	//	APlayerController* PC = CastChecked<APlayerController>(Actor);
	//	PC->GetPawnOrSpectator()->SetActorHiddenInGame(true);
	//	return false;
	//}
	return false;
}

/* Private helpers */
// Remove actor components that are not required in the 'visual only' world (e.g. controllers)
void FSLVizEpisodeReplayUtils::RemoveUnnecessaryComponents(AActor* Actor)
{
	// WARNING: anything that could cause the component to change ownership or be destroyed will invalidate this array, so use caution when iterating this set!
	// Hence the actors will be cached and removed after the iteration
	TArray<UActorComponent*> ComponentsToRemove;
	for(const auto& C : Actor->GetComponents())
	{
		if (C == Actor->GetRootComponent() 
			|| C->IsA(USLIndividualComponent::StaticClass())
			//|| C->IsA(ULightComponentBase::StaticClass())
			|| C->IsA(UMovementComponent::StaticClass())
			)
		{
			// skip
			continue;
		}
		else
		{
			ComponentsToRemove.Add(C);
		}
	}

	// Destroy cached components
	for (auto& C : ComponentsToRemove)
	{
		C->ConditionalBeginDestroy();
	}
}


//// Make sure the mesh of the pawn or spectator is not visible in the world
//void FSLVizEpisodeReplayUtils::HidePawnOrSpectator(UWorld* World)
//{
//	if (World->GetFirstPlayerController() && World->GetFirstPlayerController()->GetPawnOrSpectator())
//	{
//		World->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not access the pawn or specator to set as hidden.."),
//			*FString(__FUNCTION__), __LINE__);
//	}
//}

//// Remove unnecessary components/actors from the world for setting it up as visual only
//void FSLVizEpisodeReplayUtils::RemoveUnnecessaryActorsOrComponents(UWorld* World)
//{
//
//// Includes for removing/keeping 
//#include "Skeletal/SLSkeletalDataComponent.h"
//#include "Landscape.h"
//#include "Camera/CameraActor.h"
//#include "Camera/PlayerCameraManager.h"
//#include "Engine/PlayerStartPIE.h"
//#include "Camera/PlayerCameraManager.h"
//#include "GameFramework/PlayerController.h"
//#include "GameFramework/PlayerState.h"
//#include "GameFramework/DefaultPawn.h"
//#include "GameFramework/GameModeBase.h"
//#include "GameFramework/GameStateBase.h"
//#include "GameFramework/GameSession.h"
//#include "GameFramework/GameNetworkManager.h"
//#include "GameFramework/HUD.h"
//#include "Particles/ParticleEventManager.h"
//#include "AIController.h"

////// Blacklisted actors, remove them from the world
////if (Actor->IsA(APlayerStartPIE::StaticClass())
////	|| Actor->IsA(APlayerCameraManager::StaticClass())
////	|| Actor->IsA(APlayerState::StaticClass())
////	|| Actor->IsA(AGameStateBase::StaticClass())
////	|| Actor->IsA(AGameModeBase::StaticClass())
////	|| Actor->IsA(AGameSession::StaticClass())
////	|| Actor->IsA(AGameNetworkManager::StaticClass())
////	|| Actor->IsA(AHUD::StaticClass())
////	|| Actor->IsA(AAIController::StaticClass())
////	/*|| Actor->IsA(AParticleEventManager::StaticClass())*/
////	)
////{
////	//UE_LOG(LogTemp, Log, TEXT("%s::%d Actor %s is being removed from world.."),
////	//	*FString(__FUNCTION__), __LINE__,*Actor->GetName());
////	Actor->ConditionalBeginDestroy();
////	return;
////}
//
////// Whitelisted actors, Avoid removing landscape components, or the player controller
////if (Actor->IsA(ALandscape::StaticClass())
////	|| Actor->IsA(APlayerController::StaticClass())
////	|| Actor->IsA(ACameraActor::StaticClass())
////	|| Actor->IsA(ADefaultPawn::StaticClass())
////	|| Actor->IsA(AVizMarkerManager::StaticClass())
////	|| Actor->IsA(ASLVizEpisodeReplayManager::StaticClass())
////	)
////{
////	//UE_LOG(LogTemp, Log, TEXT("%s::%d Actor %s is whitelisted, none of its components will be removed.."),
////	//	*FString(__FUNCTION__), __LINE__,*Actor->GetName());
////	return;
////}
//
////// Remove all unnecessary components from the actors (loggers, controllers etc.)
////// Avoid "Error: Ensure condition failed: Lhs.Array.Num() == Lhs.InitialNu: Container has changed during ranged-for iteration!"
////TArray<UActorComponent*> ComponentsToDestroy;
////for (auto& C : Actor->GetComponents())
////{
////	// Whitelisted components
////	if (C->IsA(UStaticMeshComponent::StaticClass())
////		|| C->IsA(USkeletalMeshComponent::StaticClass())
////		|| C->IsA(ULightComponentBase::StaticClass())
////		/*|| C->IsA(UFloatingPawnMovement::StaticClass())*/)
////	{
////		//UE_LOG(LogTemp, Log, TEXT("%s::%d Component %s of actor %s is whitelisted, will not be removed.."),
////		//	*FString(__FUNCTION__), __LINE__, *C->GetName(), *Actor->GetName());
////		continue;
////	}
////	ComponentsToDestroy.Emplace(C);
////}
//
////// Destroy cached components
////for (auto& C : ComponentsToDestroy)
////{
////	C->ConditionalBeginDestroy();
////}
//};