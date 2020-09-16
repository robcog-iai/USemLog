// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizEpisodeReplayManager.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "ProfilerCommon.h" // FBinaryFindIndex

#if WITH_EDITOR
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/EditorEngine.h"
#endif // WITH_EDITOR

// Sets default values
ASLVizEpisodeReplayManager::ASLVizEpisodeReplayManager()
{
	// Allow ticking, disable it by default (used for episode replay)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bWorldSetAsVisualOnly = false;
	bEpisodeLoaded = false;
	bReplayShouldLoop = false;
	bReplaying = false;

	ReplayDefaultUpdateRate = 0.f;
	ReplayActiveFrameIndex = INDEX_NONE;
	ReplayFirstFrameIndex = INDEX_NONE;
	ReplayLastFrameIndex = INDEX_NONE;
	ReplayStepSize = 1;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLVizEpisodeReplayManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASLVizEpisodeReplayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// Set update rate
	//if (LoggerParameters.UpdateRate > 0.f)
	//{
	//	SetActorTickInterval(LoggerParameters.UpdateRate);
	//}
	//SetActorTickEnabled(true);
}

// Set the whole world as a visual, disable physics, collisions, attachments, unnecesary components
void ASLVizEpisodeReplayManager::SetWorldAsVisualOnly()
{
	if (bWorldSetAsVisualOnly)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d World has already been set as visual only.."),
			*FString(__FUNCTION__), __LINE__);
		return;
	}

	// Make sure the mesh of the pawn or spectator is not visible in the world
	HidePawnOrSpecator();

	// Set actors as visuals only (disable physics, set as movable, clear any attachments)
	SetActorsAsVisualsOnly();

	// Add a poseable mesh component clone to the skeletal actors
	AddPoseablMeshComponentsToSkeletalActors();

	// Remove any unnecessary actors or components
	RemoveUnnecessaryActorsOrComponents();

	// Mark world as visual only
	bWorldSetAsVisualOnly = true;
}

// Load episode data
void ASLVizEpisodeReplayManager::LoadEpisode(const FSLVizEpisodeData& InEpisodeData)
{
	// Check if the data is valid
	if (!InEpisodeData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid to load.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Stop any active replay
	if (bReplaying)
	{
		// TODO reset indexes
		// Stop replay

		ReplayActiveFrameIndex = INDEX_NONE;
		ReplayFirstFrameIndex = INDEX_NONE;
		ReplayLastFrameIndex = INDEX_NONE;
	}

	// Clear any previous episode
	EpisodeDataRaw.Clear();

	// Set the episode data
	EpisodeDataRaw = InEpisodeData;

	// Calculate a default update rate 
	int32 MaxSteps = FMath::Min(256, EpisodeDataRaw.Timestamps.Num() / 2);
	CalcDefaultUpdateRate(MaxSteps);

	bEpisodeLoaded = true;
}


//// Add a frame (make sure these are ordered)
//void ASLVizEpisodeReplayManager::AddFrame(float Timestamp,
//	const TMap<AStaticMeshActor*, FTransform>& InEntityPoses,
//	const TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>>& InSkeletalPoses)
//{
//	//if (bIsInit)
//	//{
//	//	// Add time
//	//	Timestamps.Emplace(Timestamp);
//
//	//	// Re-map to poseable mesh components
//	//	TMap<UPoseableMeshComponent*, TPair<FTransform, TMap<FString, FTransform>>> PoseableSkeletalPoses;
//	//	for (const auto& SkP : InSkeletalPoses)
//	//	{
//	//		if (UPoseableMeshComponent** PMC = SkeletalActorToPoseableMeshMap.Find(SkP.Key))
//	//		{
//	//			PoseableSkeletalPoses.Emplace(*PMC, SkP.Value);
//	//		}
//	//		else
//	//		{
//	//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no poseable mesh component created.."), *FString(__FUNCTION__), __LINE__, *SkP.Key->GetName());
//	//		}
//
//	//	}
//
//	//	// Create and add frame
//	//	FSLVizEpisodeFrame Frame;
//	//	Frame.EntityPoses = InEntityPoses;
//	//	Frame.SkeletalPoses = PoseableSkeletalPoses;
//
//	//	Frames.Emplace(Frame);
//	//}
//	//else
//	//{
//	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Init before, poseable meshes needs to be created beforeheand.."), *FString(__FUNCTION__), __LINE__);
//	//}
//}
//
//
//// Clear all frames related data, keep mappings
//void ASLVizEpisodeReplayManager::ClearFrames()
//{
//	GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//	Timestamps.Empty();
//	Frames.Empty();
//}
//
//// Goto timestamp, if timestamp is too large it goes to the last frame, if too small goes to the first frame
//void ASLVizEpisodeReplayManager::GoTo(float Timestamp)
//{
//	ReplayActiveFrameIndex = FBinaryFindIndex::LessEqual(Timestamps, Timestamp);
//	if (Frames.IsValidIndex(ReplayActiveFrameIndex))
//	{
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d Found index [%ld] is not valid.."), *FString(__FUNCTION__), __LINE__, ReplayActiveFrameIndex);
//	}
//}
//
//// Goto the next nth frame
//void ASLVizEpisodeReplayManager::Next(int32 StepSize, bool bLoop)
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
//	if (ReplayActiveFrameIndex < Frames.Num() - StepSize)
//	{
//		// Goto next step
//		ReplayActiveFrameIndex += StepSize;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else if(bLoop)
//	{
//		// Goto first frame + the remaining steps
//		ReplayActiveFrameIndex = Frames.Num() - 1 - ReplayActiveFrameIndex;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply last frame (even if it is not a full step)
//		if (ReplayActiveFrameIndex != Frames.Num() - 1)
//		{
//			ReplayActiveFrameIndex = Frames.Num() - 1;
//			Frames[ReplayActiveFrameIndex].ApplyPoses();
//		}
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, ReplayActiveFrameIndex);
//	}
//}
//
//// Goto the previous nth frame
//void ASLVizEpisodeReplayManager::Previous(int32 StepSize, bool bLoop)
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
//	if (ReplayActiveFrameIndex - StepSize >= 0)
//	{
//		// Goto previous step
//		ReplayActiveFrameIndex -= StepSize;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else if (bLoop)
//	{
//		// Goto last frame - the remaining steps
//		ReplayActiveFrameIndex = Frames.Num() - 1 - ReplayActiveFrameIndex;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply first frame (even if it is not a full step)
//		if (ReplayActiveFrameIndex != 0)
//		{
//			ReplayActiveFrameIndex = 0;
//			Frames[ReplayActiveFrameIndex].ApplyPoses();
//		}
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached first frame [%ld].."), *FString(__FUNCTION__), __LINE__, ReplayActiveFrameIndex);
//	}
//}
//
//// Replay all the episode with the given update rate (by default the update rate is calculated as the average of the first X frames)
//void ASLVizEpisodeReplayManager::Replay(int32 StepSize, float UpdateRate, bool bLoop)
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
//		ReplayActiveFrameIndex = ReplayFirstFrameIndex;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//
//		if (UpdateRate < 0.f)
//		{
//			UpdateRate = CalcDefaultUpdateRate(8);
//		}
//
//		// Use delay since the first frame was already applied
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, 0.01, true, 0.01);
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
//void ASLVizEpisodeReplayManager::Replay(float StartTime, float EndTime, int32 StepSize, float UpdateRate, bool bLoop)
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
//		ReplayActiveFrameIndex = ReplayFirstFrameIndex;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//
//		if (UpdateRate < 0.f)
//		{
//			UpdateRate = CalcDefaultUpdateRate(8);
//		}
//
//		// Use delay since the first frame was already applied
//		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, UpdateRate, true, UpdateRate);
//	}
//	else
//	{
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Previous replay was already running, restarting.."), *FString(__FUNCTION__), __LINE__);
//		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//		Replay(StartTime, EndTime, UpdateRate, bLoop);
//	}
//}
//
//// Pause / start replay
//void ASLVizEpisodeReplayManager::ToggleReplay()
//{
//	if (ReplayTimerHandle.IsValid())
//	{
//		if (GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
//		}
//		else
//		{
//			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
//	}
//}
//
//// Set replay to pause or play
//void ASLVizEpisodeReplayManager::SetPauseReplay(bool bPause)
//{
//	if (ReplayTimerHandle.IsValid())
//	{
//		if (bPause && GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
//		}
//		else if (!bPause && GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
//		{
//			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
//	}
//}
//
//// Replay timer callback
//void ASLVizEpisodeReplayManager::TimerCallback()
//{
//	if (ReplayActiveFrameIndex <= ReplayLastFrameIndex - ReplayStepSize)
//	{
//		// Goto next step
//		ReplayActiveFrameIndex += ReplayStepSize;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else if (bLoopTheReplay)
//	{
//		// Goto first frame + the remaining steps
//		ReplayActiveFrameIndex = ReplayLastFrameIndex - ReplayActiveFrameIndex;
//		Frames[ReplayActiveFrameIndex].ApplyPoses();
//	}
//	else
//	{
//		// Apply last frame (even if it is not a full step)
//		if (ReplayActiveFrameIndex != ReplayLastFrameIndex)
//		{
//			ReplayActiveFrameIndex = ReplayLastFrameIndex;
//			Frames[ReplayActiveFrameIndex].ApplyPoses();
//		}
//		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
//		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, ReplayActiveFrameIndex);
//	}
//}


// Make sure the mesh of the pawn or spectator is not visible in the world
void ASLVizEpisodeReplayManager::HidePawnOrSpecator()
{
	if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator())
	{
		GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not access the pawn or specator to set as hidden.."),
			*FString(__FUNCTION__), __LINE__);
	}
}

// Set actors as visuals only (disable physics, set as movable, clear attachments)
void ASLVizEpisodeReplayManager::SetActorsAsVisualsOnly()
{
	// Iterate all actors
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
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
	}
}

// Add a poseable mesh component clone to the skeletal actors
void ASLVizEpisodeReplayManager::AddPoseablMeshComponentsToSkeletalActors()
{
	for (TActorIterator<ASkeletalMeshActor> SkelActItr(GetWorld()); SkelActItr; ++SkelActItr)
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

// Remove any unnecessary actors or components
void ASLVizEpisodeReplayManager::RemoveUnnecessaryActorsOrComponents()
{
	// !!  Done in SemLog !!

//// Blacklisted actors, remove them from the world
//if (Actor->IsA(APlayerStartPIE::StaticClass())
//	|| Actor->IsA(APlayerCameraManager::StaticClass())
//	|| Actor->IsA(APlayerState::StaticClass())
//	|| Actor->IsA(AGameStateBase::StaticClass())
//	|| Actor->IsA(AGameModeBase::StaticClass())
//	|| Actor->IsA(AGameSession::StaticClass())
//	|| Actor->IsA(AGameNetworkManager::StaticClass())
//	|| Actor->IsA(AHUD::StaticClass())
//	|| Actor->IsA(AAIController::StaticClass())
//	/*|| Actor->IsA(AParticleEventManager::StaticClass())*/
//	)
//{
//	//UE_LOG(LogTemp, Log, TEXT("%s::%d Actor %s is being removed from world.."),
//	//	*FString(__FUNCTION__), __LINE__,*Actor->GetName());
//	Actor->ConditionalBeginDestroy();
//	return;
//}

//// Whitelisted actors, Avoid removing landscape components, or the player controller
//if (Actor->IsA(ALandscape::StaticClass())
//	|| Actor->IsA(APlayerController::StaticClass())
//	|| Actor->IsA(ACameraActor::StaticClass())
//	|| Actor->IsA(ADefaultPawn::StaticClass())
//	|| Actor->IsA(AVizMarkerManager::StaticClass())
//	|| Actor->IsA(ASLVizEpisodeReplayManager::StaticClass())
//	)
//{
//	//UE_LOG(LogTemp, Log, TEXT("%s::%d Actor %s is whitelisted, none of its components will be removed.."),
//	//	*FString(__FUNCTION__), __LINE__,*Actor->GetName());
//	return;
//}

//// Remove all unnecessary components from the actors (loggers, controllers etc.)
//// Avoid "Error: Ensure condition failed: Lhs.Array.Num() == Lhs.InitialNu: Container has changed during ranged-for iteration!"
//TArray<UActorComponent*> ComponentsToDestroy;
//for (auto& C : Actor->GetComponents())
//{
//	// Whitelisted components
//	if (C->IsA(UStaticMeshComponent::StaticClass())
//		|| C->IsA(USkeletalMeshComponent::StaticClass())
//		|| C->IsA(ULightComponentBase::StaticClass())
//		/*|| C->IsA(UFloatingPawnMovement::StaticClass())*/)
//	{
//		//UE_LOG(LogTemp, Log, TEXT("%s::%d Component %s of actor %s is whitelisted, will not be removed.."),
//		//	*FString(__FUNCTION__), __LINE__, *C->GetName(), *Actor->GetName());
//		continue;
//	}
//	ComponentsToDestroy.Emplace(C);
//}

//// Destroy cached components
//for (auto& C : ComponentsToDestroy)
//{
//	C->ConditionalBeginDestroy();
//}
}

// Calculate an average update rate from the timestamps
void ASLVizEpisodeReplayManager::CalcDefaultUpdateRate(int32 MaxNumSteps)
{
	if (!EpisodeDataRaw.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Episode data is not valid, cannot aprox a default update rate"),
			*FString(__FUNCTION__), __LINE__);
		ReplayDefaultUpdateRate = 0.f;
	}

	// Make an average value using the MaxNumSteps entries from the first quarter
	int32 StartFrameIdx = EpisodeDataRaw.Timestamps.Num() / 4;
	int32 EndFrameIdx = StartFrameIdx + MaxNumSteps;
	float UpdateRate = 0.f;
	if (EndFrameIdx > EpisodeDataRaw.Timestamps.Num())
	{
		EndFrameIdx = EpisodeDataRaw.Timestamps.Num();
	}

	// Start from the first quarter, at the beginning one might have some outliers due to loading time spikes
	for (int32 Idx = StartFrameIdx; Idx < EndFrameIdx - 1; ++Idx)
	{
		UpdateRate += (EpisodeDataRaw.Timestamps[Idx+1] - EpisodeDataRaw.Timestamps[Idx]);
	}

	ReplayDefaultUpdateRate = UpdateRate / ((float)(MaxNumSteps - 1));
}
