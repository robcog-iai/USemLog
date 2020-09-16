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
	PrimaryActorTick.bCanEverTick = false;
	bIsInit = false;
	CurrFrameIdx = INDEX_NONE;
	ReplayFirstFrameIdx = INDEX_NONE;
	ReplayLastFrameIdx = INDEX_NONE;
	ReplayStepSize = 1;
	bReplayLoop = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Set the whole world as a visual, disable physics, collisions, attachments, unnecesary components
void ASLVizEpisodeReplayManager::SetWorldAsVisualOnly()
{
	// Hide default pawn
	if (GetWorld()->GetFirstPlayerController())
	{
		GetWorld()->GetFirstPlayerController()->GetPawnOrSpectator()->SetActorHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Default pawn/spectator remained visible.."), *FString(__FUNCTION__), __LINE__);
	}

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
		TArray<AActor*> AttachedActors;
		ActItr->GetAttachedActors(AttachedActors);
		for (auto& AttAct : AttachedActors)
		{
			AttAct->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}

		// Create poseable meshes if actor is skeletal
		if (ASkeletalMeshActor* SkMA = Cast<ASkeletalMeshActor>(*ActItr))
		{
			SkeletalActorToPoseableMeshMap.Add(SkMA, CreateNewPoseableMeshComponent(SkMA));
		}

		// Remove components, or the actor itself if not required in the world
		//ShouldActorBeRemoved(*ActItr);
	}
}

// Called when the game starts or when spawned
void ASLVizEpisodeReplayManager::BeginPlay()
{
	Super::BeginPlay();
	Init();
}

#if WITH_EDITOR
void ASLVizEpisodeReplayManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLVizEpisodeReplayManager, bExecuteInitButtonHack))
	{
		bExecuteInitButtonHack = false;
		Init();
	}

	TMap<float, FString> Map;

	TArray<float> Arr;



	
}
#endif // WITH_EDITOR

// Called when actor removed from game or game ended
void ASLVizEpisodeReplayManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Set world to visual only, create poseable skeletal mesh components
void ASLVizEpisodeReplayManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Replay manager (%s) is already init.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

#if WITH_EDITOR
	
	// True if we are in the editor.
	if (!GIsEditor)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Replay manager (%s) is not in the editor.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}

	// GEditor->PlayWorld: A pointer to a UWorld that is the duplicated/saved-loaded to be played in with "Play From Here"
	if (GEditor->PlayWorld || GIsPlayInEditorWorld)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Replay manager (%s): the editor is currently in a play mode.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
#endif // WITH_EDITOR
	
	SetWorldAsVisualOnly();
	bIsInit = true;	
}


// Add a frame (make sure these are ordered)
void ASLVizEpisodeReplayManager::AddFrame(float Timestamp,
	const TMap<AStaticMeshActor*, FTransform>& InEntityPoses,
	const TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>>& InSkeletalPoses)
{
	//if (bIsInit)
	//{
	//	// Add time
	//	Timestamps.Emplace(Timestamp);

	//	// Re-map to poseable mesh components
	//	TMap<UPoseableMeshComponent*, TPair<FTransform, TMap<FString, FTransform>>> PoseableSkeletalPoses;
	//	for (const auto& SkP : InSkeletalPoses)
	//	{
	//		if (UPoseableMeshComponent** PMC = SkeletalActorToPoseableMeshMap.Find(SkP.Key))
	//		{
	//			PoseableSkeletalPoses.Emplace(*PMC, SkP.Value);
	//		}
	//		else
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no poseable mesh component created.."), *FString(__FUNCTION__), __LINE__, *SkP.Key->GetName());
	//		}

	//	}

	//	// Create and add frame
	//	FSLVizEpisodeFrame Frame;
	//	Frame.EntityPoses = InEntityPoses;
	//	Frame.SkeletalPoses = PoseableSkeletalPoses;

	//	Frames.Emplace(Frame);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d Init before, poseable meshes needs to be created beforeheand.."), *FString(__FUNCTION__), __LINE__);
	//}
}


// Clear all frames related data, keep mappings
void ASLVizEpisodeReplayManager::ClearFrames()
{
	GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
	Timestamps.Empty();
	Frames.Empty();
}

// Goto timestamp, if timestamp is too large it goes to the last frame, if too small goes to the first frame
void ASLVizEpisodeReplayManager::GoTo(float Timestamp)
{
	CurrFrameIdx = FBinaryFindIndex::LessEqual(Timestamps, Timestamp);
	if (Frames.IsValidIndex(CurrFrameIdx))
	{
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Found index [%ld] is not valid.."), *FString(__FUNCTION__), __LINE__, CurrFrameIdx);
	}
}

// Goto the next nth frame
void ASLVizEpisodeReplayManager::Next(int32 StepSize, bool bLoop)
{	
	if (Frames.Num() < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (StepSize > Frames.Num() / 2)
	{
		StepSize = Frames.Num() / 2;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
	}

	if (CurrFrameIdx < Frames.Num() - StepSize)
	{
		// Goto next step
		CurrFrameIdx += StepSize;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else if(bLoop)
	{
		// Goto first frame + the remaining steps
		CurrFrameIdx = Frames.Num() - 1 - CurrFrameIdx;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else
	{
		// Apply last frame (even if it is not a full step)
		if (CurrFrameIdx != Frames.Num() - 1)
		{
			CurrFrameIdx = Frames.Num() - 1;
			Frames[CurrFrameIdx].ApplyPoses();
		}
		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, CurrFrameIdx);
	}
}

// Goto the previous nth frame
void ASLVizEpisodeReplayManager::Previous(int32 StepSize, bool bLoop)
{
	if (Frames.Num() < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available.."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	if (StepSize > Frames.Num() / 2)
	{
		StepSize = Frames.Num() / 2;
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
	}

	if (CurrFrameIdx - StepSize >= 0)
	{
		// Goto previous step
		CurrFrameIdx -= StepSize;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else if (bLoop)
	{
		// Goto last frame - the remaining steps
		CurrFrameIdx = Frames.Num() - 1 - CurrFrameIdx;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else
	{
		// Apply first frame (even if it is not a full step)
		if (CurrFrameIdx != 0)
		{
			CurrFrameIdx = 0;
			Frames[CurrFrameIdx].ApplyPoses();
		}
		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached first frame [%ld].."), *FString(__FUNCTION__), __LINE__, CurrFrameIdx);
	}
}

// Replay all the episode with the given update rate (by default the update rate is calculated as the average of the first X frames)
void ASLVizEpisodeReplayManager::Replay(int32 StepSize, float UpdateRate, bool bLoop)
{
	if (Frames.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available to replay.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
	if (!GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
	{
		if (StepSize > Frames.Num() / 2)
		{
			StepSize = Frames.Num() / 2;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
		}

		bReplayLoop = bLoop;
		ReplayStepSize = StepSize;
		ReplayFirstFrameIdx = 0;		
		ReplayLastFrameIdx = Frames.Num() - 1;

		// Apply first frame
		CurrFrameIdx = ReplayFirstFrameIdx;
		Frames[CurrFrameIdx].ApplyPoses();

		if (UpdateRate < 0.f)
		{
			UpdateRate = GetReplayUpdateRateFromTimestampsAverage(8);
		}

		// Use delay since the first frame was already applied
		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, 0.01, true, 0.01);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Previous replay was already running, restarting.."), *FString(__FUNCTION__), __LINE__);
		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
		Replay(UpdateRate, bLoop);
	}
}

// Replay the episode between the timestamp with the given update rate (by default the update rate is calculated as the average of the first X frames)
void ASLVizEpisodeReplayManager::Replay(float StartTime, float EndTime, int32 StepSize, float UpdateRate, bool bLoop)
{
	if (Frames.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Not enough frames available to replay.."), *FString(__FUNCTION__), __LINE__);
		return;
	}
	if (!GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
	{
		if (StepSize > Frames.Num() / 2)
		{
			StepSize = Frames.Num() / 2;
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Step size too large, decreased to Frames.Num() / 2 = %ld;"), *FString(__FUNCTION__), __LINE__, StepSize);
		}

		bReplayLoop = bLoop;
		ReplayStepSize = StepSize;
		ReplayFirstFrameIdx = FBinaryFindIndex::LessEqual(Timestamps, StartTime);		
		ReplayLastFrameIdx = FBinaryFindIndex::LessEqual(Timestamps, EndTime);

		// Apply first frame
		CurrFrameIdx = ReplayFirstFrameIdx;
		Frames[CurrFrameIdx].ApplyPoses();

		if (UpdateRate < 0.f)
		{
			UpdateRate = GetReplayUpdateRateFromTimestampsAverage(8);
		}

		// Use delay since the first frame was already applied
		GetWorld()->GetTimerManager().SetTimer(ReplayTimerHandle, this, &ASLVizEpisodeReplayManager::TimerCallback, UpdateRate, true, UpdateRate);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Previous replay was already running, restarting.."), *FString(__FUNCTION__), __LINE__);
		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
		Replay(StartTime, EndTime, UpdateRate, bLoop);
	}
}

// Pause / start replay
void ASLVizEpisodeReplayManager::ToggleReplay()
{
	if (ReplayTimerHandle.IsValid())
	{
		if (GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
		{
			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
		}
		else
		{
			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Set replay to pause or play
void ASLVizEpisodeReplayManager::SetPauseReplay(bool bPause)
{
	if (ReplayTimerHandle.IsValid())
	{
		if (bPause && GetWorld()->GetTimerManager().IsTimerActive(ReplayTimerHandle))
		{
			GetWorld()->GetTimerManager().PauseTimer(ReplayTimerHandle);
		}
		else if (!bPause && GetWorld()->GetTimerManager().IsTimerPaused(ReplayTimerHandle))
		{
			GetWorld()->GetTimerManager().UnPauseTimer(ReplayTimerHandle);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No replay is running, nothing to start/pause.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Replay timer callback
void ASLVizEpisodeReplayManager::TimerCallback()
{
	if (CurrFrameIdx <= ReplayLastFrameIdx - ReplayStepSize)
	{
		// Goto next step
		CurrFrameIdx += ReplayStepSize;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else if (bReplayLoop)
	{
		// Goto first frame + the remaining steps
		CurrFrameIdx = ReplayLastFrameIdx - CurrFrameIdx;
		Frames[CurrFrameIdx].ApplyPoses();
	}
	else
	{
		// Apply last frame (even if it is not a full step)
		if (CurrFrameIdx != ReplayLastFrameIdx)
		{
			CurrFrameIdx = ReplayLastFrameIdx;
			Frames[CurrFrameIdx].ApplyPoses();
		}
		GetWorld()->GetTimerManager().ClearTimer(ReplayTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("%s::%d Reached last frame [%ld].."), *FString(__FUNCTION__), __LINE__, CurrFrameIdx);
	}
}



// Check if actor or any of its components should be removed
void ASLVizEpisodeReplayManager::ShouldActorBeRemoved(AActor* Actor) const
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

// Hide skeletal mesh components and create new visible poseable mesh components
UPoseableMeshComponent* ASLVizEpisodeReplayManager::CreateNewPoseableMeshComponent(ASkeletalMeshActor* SkeletalActor) const
{
	UPoseableMeshComponent* PMC = NewObject<UPoseableMeshComponent>(SkeletalActor);
	PMC->SetSkeletalMesh(SkeletalActor->GetSkeletalMeshComponent()->SkeletalMesh);
	PMC->AttachToComponent(SkeletalActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PMC->RegisterComponent();
	PMC->bHasMotionBlurVelocityMeshes = false;
	PMC->bPerBoneMotionBlur = false;
	SkeletalActor->AddInstanceComponent(PMC); // Makes it appear in the editor
	SkeletalActor->GetSkeletalMeshComponent()->SetHiddenInGame(true);
	return PMC;
}

// Calculate an average update rate from the timestamps
float ASLVizEpisodeReplayManager::GetReplayUpdateRateFromTimestampsAverage(int32 Steps) const
{
	// Make an average value using the frist X timestamps
	int32 StartFrameIdx = Timestamps.Num() / 2;
	int32 EndFrameIdx = StartFrameIdx + Steps;
	float UpdateRate = 0.f;
	if (EndFrameIdx > Timestamps.Num())
	{
		EndFrameIdx = Timestamps.Num();
	}
	// Start from the middle, at the beginning you might get outliers
	for (int32 Idx = StartFrameIdx; Idx < EndFrameIdx - 1; ++Idx)
	{
		UpdateRate += (Timestamps[Idx+1] - Timestamps[Idx]);
	}
	return UpdateRate / ((float)(Steps - 1));
}
