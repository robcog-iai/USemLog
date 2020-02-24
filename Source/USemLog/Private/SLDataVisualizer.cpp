// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLDataVisualizer.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "EngineUtils.h"
#include "SLEntitiesManager.h"

#if SL_WITH_DATA_VIS
// Clean world White-/black-listed components/actors
#include "Skeletal/SLSkeletalDataComponent.h"
#include "Landscape.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/PlayerStartPIE.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/HUD.h"
#include "Particles/ParticleEventManager.h"
#include "AIController.h"
#endif //SL_WITH_DATA_VIS

// Constructor
USLDataVisualizer::USLDataVisualizer() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	QueryIdx = INDEX_NONE;
}

// Destructor
USLDataVisualizer::~USLDataVisualizer()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init Logger
void USLDataVisualizer::Init(USLDataVisQueries* InQueries)
{
	if (!bIsInit)
	{
		VisQueries = InQueries;

		// Remove unnecesary actors/components from the world
		CleanWorld();

		// Spawn marker viz manager 
		if (!SpawnVizMarkerManager())
		{
			return;
		}

		// Spawn world viz manager
		if (!SpawnVizWorldManager())
		{
			return;
		}
	
#if SL_WITH_DATA_VIS
		/* Check connection to database */
		if (!QAHandler.ConnectToServer(VisQueries->ServerIP, VisQueries->ServerPort))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Query handler not connect to server.."), *FString(__func__), __LINE__);
			return;
		}
#endif //SL_WITH_DATA_VIS
		
		bIsInit = true;	
	}
}

// Start logger
void USLDataVisualizer::Start(const FName& UserInputActionName)
{
	if (!bIsStarted && bIsInit)
	{
#if SL_WITH_DATA_VIS
		if (VizWorldManager)
		{			
			TArray<ASkeletalMeshActor*> SkeletalActors;
			FSLEntitiesManager::GetInstance()->GetSkeletalMeshActors(SkeletalActors);
			
			// Set world to visual only + create poseable mesh components for the skeletal actors (hide original skeletal components)
			VizWorldManager->Setup();
		}
#endif //SL_WITH_DATA_VIS

		if (SetupUserInput(UserInputActionName))
		{
			bIsStarted = true;
		}		
	}
}

// Finish logger
void USLDataVisualizer::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
#if SL_WITH_DATA_VIS
		QAHandler.Disconnect();
#endif //SL_WITH_DATA_VIS

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Setup the user input bindings
bool USLDataVisualizer::SetupUserInput(const FName& UserInputActionName)
{
	// Set user input bindings
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(UserInputActionName, IE_Released, this, &USLDataVisualizer::Query);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No Input Component found.."), *FString(__func__), __LINE__);
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No Player Controller found.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Visualize current query
void USLDataVisualizer::Query()
{
#if SL_WITH_DATA_VIS	
	QueryIdx++;

	if (VisQueries->Queries.IsValidIndex(QueryIdx))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld];"), *FString(__FUNCTION__), __LINE__, QueryIdx);

		/* Connection */
		const FString DBName = VisQueries->Queries[QueryIdx].TaskId;
		if (!PrevDBName.Equals(DBName))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting database to %s.."), *FString(__func__), __LINE__, *DBName);
			if (!QAHandler.SetDatabase(DBName))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not set database to %s, aborting query.."),
					*FString(__func__), __LINE__, *DBName);
				return;
			}
			PrevDBName = DBName;
		}

		const FString CollName = VisQueries->Queries[QueryIdx].EpisodeId;
		if (!PrevCollName.Equals(CollName))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting collection to %s.."), *FString(__func__), __LINE__, *CollName);
			if (!QAHandler.SetCollection(CollName))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not set collection to %s, aborting query.."),
					*FString(__func__), __LINE__, *FString(DBName + "." + CollName));
				return;
			}
			PrevCollName = CollName;

			// Pre-load world states
			TArray<FMQWorldStateFrame> WorldStates;
			if (QAHandler.GetAllWorldStates(WorldStates))
			{
				PreLoadWorldStates(WorldStates);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d WorldState query for pre-load failed.."), *FString(__FUNCTION__), __LINE__);
			}
		}

		/* Query */
		switch (VisQueries->Queries[QueryIdx].QueryType)
		{
		case ESLVisQueryType::EntityPose:
		{
			EntityPoseQuery();
			break;
		}
		case ESLVisQueryType::EntityTraj:
		{
			EntityTrajQuery();
			break;
		}
		case ESLVisQueryType::BonePose:
		{
			BonePoseQuery();
			break;
		}
		case ESLVisQueryType::BoneTraj:
		{
			BoneTrajQuery();
			break;
		}
		case ESLVisQueryType::SkeletalPose:
		{
			SkelPoseQuery();
			break;
		}
		case ESLVisQueryType::SkeletalTraj:
		{
			SkelTrajQuery();
			break;
		}
		case ESLVisQueryType::GazePose:
		{
			GazePoseQuery();
			break;
		}
		case ESLVisQueryType::GazeTraj:
		{
			GazeTrajQuery();
			break;
		}
		case ESLVisQueryType::WorldState:
		{
			WorldStateQuery();
			break;
		}
		case ESLVisQueryType::AllWorldStates:
		{
			AllWorldStatesQuery();
			break;
		}

		default:
			UE_LOG(LogTemp, Error, TEXT("%s::%d Unknown query type.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No more queries.."), *FString(__func__), __LINE__);
		VizMarkerManager->ClearAllMarkers();
	}
#endif //SL_WITH_DATA_VIS
}

// Clean world from unnecesary actors/components
void USLDataVisualizer::CleanWorld()
{
#if SL_WITH_DATA_VIS
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Blacklisted actors, remove them from the world
		if (ActItr->IsA(APlayerStartPIE::StaticClass())
			|| ActItr->IsA(APlayerCameraManager::StaticClass())
			|| ActItr->IsA(APlayerState::StaticClass())
			|| ActItr->IsA(AGameStateBase::StaticClass())
			|| ActItr->IsA(AGameModeBase::StaticClass())
			|| ActItr->IsA(AGameSession::StaticClass())
			|| ActItr->IsA(AGameNetworkManager::StaticClass())
			|| ActItr->IsA(AHUD::StaticClass())
			|| ActItr->IsA(AAIController::StaticClass())
			)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d ActItr %s is being removed from world.."),
				*FString(__FUNCTION__), __LINE__,*ActItr->GetName());
			ActItr->ConditionalBeginDestroy();
			continue;
		}

		// Whitelisted ActItrs, Avoid removing landscape components, or the player controller
		if (ActItr->IsA(ALandscape::StaticClass())
			|| ActItr->IsA(APlayerController::StaticClass())
			|| ActItr->IsA(ACameraActor::StaticClass())
			|| ActItr->IsA(ADefaultPawn::StaticClass())
			|| ActItr->IsA(AVizMarkerManager::StaticClass())
			|| ActItr->IsA(AVizWorldManager::StaticClass())
			)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d ActItr %s is whitelisted, none of its components will be removed.."),
				*FString(__FUNCTION__), __LINE__,*ActItr->GetName());
			continue;
		}

		// Remove all unnecessary components from the ActItrs (loggers, controllers etc.)
		// Avoid "Error: Ensure condition failed: Lhs.Array.Num() == Lhs.InitialNu: Container has changed during ranged-for iteration!"
		TArray<UActorComponent*> ComponentsToDestroy;
		for (auto& C : ActItr->GetComponents())
		{
			// Whitelisted components
			if (C->IsA(UStaticMeshComponent::StaticClass())
				|| C->IsA(USkeletalMeshComponent::StaticClass())
				|| C->IsA(ULightComponentBase::StaticClass())
				|| C->IsA(USLSkeletalDataComponent::StaticClass())
				)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s of ActItr %s is whitelisted, will not be removed.."),
					*FString(__FUNCTION__), __LINE__, *C->GetName(), *ActItr->GetName());
				continue;
			}
			ComponentsToDestroy.Emplace(C);
		}

		// Destroy cached components
		for (auto& C : ComponentsToDestroy)
		{
			C->ConditionalBeginDestroy();
		}
	}
#endif //SL_WITH_DATA_VIS
}

// Spawn marker viz manager actor
bool USLDataVisualizer::SpawnVizMarkerManager()
{
#if SL_WITH_DATA_VIS
	FActorSpawnParameters VizMarkerSpawnParams;
	VizMarkerSpawnParams.Name = TEXT("SL_VizMarkerManager");
	VizMarkerManager = GetWorld()->SpawnActor<AVizMarkerManager>(AVizMarkerManager::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector::ZeroVector), VizMarkerSpawnParams);
	if (!VizMarkerManager)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn the MarkerManager.."), *FString(__func__), __LINE__);
		return false;
	}
	VizMarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));
	return true;
#else
	return false;
#endif //SL_WITH_DATA_VIS
}

// Spawn world viz manager actor
bool USLDataVisualizer::SpawnVizWorldManager()
{
#if SL_WITH_DATA_VIS
	FActorSpawnParameters VizWorldSpawnParams;
	VizWorldSpawnParams.Name = TEXT("SL_VizWorldManager");
	VizWorldManager = GetWorld()->SpawnActor<AVizWorldManager>(AVizWorldManager::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector::ZeroVector), VizWorldSpawnParams);
	if (!VizWorldManager)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn the MarkerManager.."), *FString(__func__), __LINE__);
		return false;
	}
	VizWorldManager->SetActorLabel(TEXT("SL_VizWorldManager"));
	return true;
#else
	return false;
#endif //SL_WITH_DATA_VIS
}

#if SL_WITH_DATA_VIS
// Pre-load workl states for the selected episode
void USLDataVisualizer::PreLoadWorldStates(const TArray<FMQWorldStateFrame>& WorldStates)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Pre-loading world states.."), *FString(__FUNCTION__), __LINE__);

	// Clear any previous frames
	VizWorldManager->ClearFrames();
	
	// Gives us the ids to actors mappings
	const auto EntitiesMgr = FSLEntitiesManager::GetInstance();

	// Set up episode in the world manager
	for (const auto& Frame : WorldStates)
	{
		// Re-create map using static mesh actor pointers
		TMap<AStaticMeshActor*, FTransform> SMAPoses;
		for (const auto& SMIdToPosePair : Frame.EntityPoses)
		{
			if (AStaticMeshActor* SMA = EntitiesMgr->GetStaticMeshActor(SMIdToPosePair.Key))
			{
				SMAPoses.Emplace(SMA, SMIdToPosePair.Value);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find static mesh actor with id=%s, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *SMIdToPosePair.Key);
			}
		}

		// Re-create map using skeletal mesh actor pointers
		TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>> SkMAPoses;
		for (const auto& SkMIdToPosePair : Frame.SkeletalPoses)
		{
			if (ASkeletalMeshActor* SkMA = EntitiesMgr->GetSkeletalMeshActor(SkMIdToPosePair.Key))
			{
				SkMAPoses.Emplace(SkMA, SkMIdToPosePair.Value);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find skeletal mesh actor with id=%s, this should not happen.."),
					*FString(__FUNCTION__), __LINE__, *SkMIdToPosePair.Key);
			}
		}

		// Add frame to the world manager
		VizWorldManager->AddFrame(Frame.Timestamp, SMAPoses, SkMAPoses);
	}
}
#endif //SL_WITH_DATA_VIS

/* Query cases */
void USLDataVisualizer::EntityPoseQuery()
{
#if SL_WITH_DATA_VIS
	FTransform Pose;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	float Ts = VisQueries->Queries[QueryIdx].StartTimestamp;
	if (QAHandler.GetEntityPoseAt(Id, Ts, Pose))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityPose \t [%s:%f] :\t T=[%s];"),
			*FString(__FUNCTION__), __LINE__, *Id, Ts, *Pose.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d EntityPose query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::EntityTrajQuery()
{
#if SL_WITH_DATA_VIS
	TArray<FTransform> Traj;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	float StartTime = VisQueries->Queries[QueryIdx].StartTimestamp;
	float EndTime = VisQueries->Queries[QueryIdx].EndTimestamp;
	float DeltaT = VisQueries->Queries[QueryIdx].DeltaT;
	if (QAHandler.GetEntityTrajectory(Id, StartTime, EndTime, Traj, DeltaT))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityTraj \t [%s:%f-%f:%f] :\t TNum=[%ld];"),
			*FString(__FUNCTION__), __LINE__, *Id, StartTime, EndTime, DeltaT, Traj.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d EntityTraj query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::BonePoseQuery()
{
#if SL_WITH_DATA_VIS
	FTransform Pose;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	FString BoneName = VisQueries->Queries[QueryIdx].BoneName;
	float Ts = VisQueries->Queries[QueryIdx].StartTimestamp;
	if (QAHandler.GetBonePoseAt(Id,	BoneName, Ts, Pose))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d BonePose \t [%s-%s:%f] :\t T=[%s];"),
			*FString(__FUNCTION__), __LINE__, *Id, *BoneName, Ts, *Pose.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d BonePose query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::BoneTrajQuery()
{
#if SL_WITH_DATA_VIS
	TArray<FTransform> Traj;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	FString BoneName = VisQueries->Queries[QueryIdx].BoneName;
	float StartTime = VisQueries->Queries[QueryIdx].StartTimestamp;
	float EndTime = VisQueries->Queries[QueryIdx].EndTimestamp;
	float DeltaT = VisQueries->Queries[QueryIdx].DeltaT;
	if (QAHandler.GetBoneTrajectory(Id, BoneName, StartTime, EndTime, Traj, DeltaT))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d BoneTraj \t [%s-%s:%f-%f:%f] :\t TNum=[%ld];"),
			*FString(__FUNCTION__), __LINE__, *Id, *BoneName, StartTime, EndTime, DeltaT, Traj.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d BoneTraj query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::SkelPoseQuery()
{
#if SL_WITH_DATA_VIS
	TPair<FTransform, TMap<FString, FTransform>> SkeletalPose;
	FTransform Pose;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	float Ts = VisQueries->Queries[QueryIdx].StartTimestamp;
	if (QAHandler.GetSkeletalPoseAt(Id, Ts, SkeletalPose))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d SkeletalPose \t [%s:%f] :\t T=[%s]; TBoneNum=[%ld];"),
			*FString(__FUNCTION__), __LINE__, *Id, Ts, *SkeletalPose.Key.ToString(), SkeletalPose.Value.Num());

		//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor(Id))
		//{
		//	FLinearColor Col = FLinearColor::Blue;
		//	Col.A = 0.3;
		//	VizMarkerManager->CreateMarker(SkelPose, SkMA->GetSkeletalMeshComponent(), 2, false, Col);
		//	UE_LOG(LogTemp, Error, TEXT(" !!!! Skeletal pose created"));
		//}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d SkeletalPose query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::SkelTrajQuery()
{
#if SL_WITH_DATA_VIS
	TArray<TPair<FTransform, TMap<FString, FTransform>>> SkeletalTraj;
	FString Id = VisQueries->Queries[QueryIdx].EntityId;
	float StartTime = VisQueries->Queries[QueryIdx].StartTimestamp;
	float EndTime = VisQueries->Queries[QueryIdx].EndTimestamp;
	float DeltaT = VisQueries->Queries[QueryIdx].DeltaT;
	if (QAHandler.GetSkeletalTrajectory(Id, StartTime, EndTime, SkeletalTraj, DeltaT))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d SkeletalTraj \t [%s:%f-%f:%f] :\t TNum=[%ld];"),
			*FString(__FUNCTION__), __LINE__, *Id, StartTime, EndTime, DeltaT, SkeletalTraj.Num());

		//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor(VisQueries->Queries[QueryIdx].EntityId))
		//{
		//	FLinearColor Col = FLinearColor::Red;
		//	Col.A = 0.3;
		//	VizMarkerManager->CreateMarker(SkeletalTraj, SkMA->GetSkeletalMeshComponent(), 5, false, Col);

		//	UE_LOG(LogTemp, Log, TEXT(" !!!! Skeletal Traj created"));
		//}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d SkeletalTraj query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::GazePoseQuery()
{
#if SL_WITH_DATA_VIS
	FVector Target;
	FVector Origin;
	float Ts = VisQueries->Queries[QueryIdx].StartTimestamp;
	if (QAHandler.GetGazePose(Ts, Target, Origin))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d GazePose \t [%f] :\t Target=[%s]; Origin=[%s]"),
			*FString(__FUNCTION__), __LINE__, Ts, *Target.ToString(), *Origin.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d GazePose query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::GazeTrajQuery()
{
#if SL_WITH_DATA_VIS
	TArray<FVector> Targets;
	TArray<FVector> Origins;
	float StartTime = VisQueries->Queries[QueryIdx].StartTimestamp;
	float EndTime = VisQueries->Queries[QueryIdx].EndTimestamp;
	float DeltaT = VisQueries->Queries[QueryIdx].DeltaT;

	if (QAHandler.GetGazeTrajectory(StartTime, EndTime, Targets, Origins, DeltaT))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d GazeTraj \t [%f-%f:%f] :\t TargetNum=[%ld]; OriginNum=[%ld]"),
			*FString(__FUNCTION__), __LINE__, StartTime, EndTime, DeltaT, Targets.Num(), Origins.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d GazeTraj query failed.."), *FString(__FUNCTION__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::WorldStateQuery()
{
#if SL_WITH_DATA_VIS
	//TMap<FString, FTransform> Entities;
	//TMap<FString, TPair<FTransform, TMap<FString, FTransform>>> Skeletals;
	//if (QAHandler.GetWorldStateAt(VisQueries->Queries[QueryIdx].StartTimestamp, Entities, Skeletals))
	//{
	//	WorldStateResult(VisQueries->Queries[QueryIdx].StartTimestamp, Entities, Skeletals);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d WorldState query failed.."), *FString(__FUNCTION__), __LINE__);
	//}
	if (VizWorldManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d WorldState query, goto ts.."), *FString(__FUNCTION__), __LINE__);
		VizWorldManager->GoTo(VisQueries->Queries[QueryIdx].StartTimestamp);
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::AllWorldStatesQuery()
{
#if SL_WITH_DATA_VIS
	//TArray<FMQWorldStateFrame> WorldStates;
	//if (QAHandler.GetAllWorldStates(WorldStates))
	//{
	//	AllWorldStatesResult(WorldStates);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d WorldState query failed.."), *FString(__FUNCTION__), __LINE__);
	//}
	if (VizWorldManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d AllWorldStates query, starting replay.."), *FString(__FUNCTION__), __LINE__);
		VizWorldManager->Replay();
	}
#endif //SL_WITH_DATA_VIS
}

// TESTS
void USLDataVisualizer::SMTest()
{
#if SL_WITH_DATA_VIS
	UE_LOG(LogTemp, Warning, TEXT("%s::%d .."), *FString(__func__), __LINE__);

	if (AStaticMeshActor* DoorSMA = FSLEntitiesManager::GetInstance()->GetStaticMeshActor("zUDyz+WsLE6Onk4Nv5lhzg"))
	{
		TArray<FVector> SMLocations;
		for (uint32 Idx = 0; Idx < 10; Idx++)
		{
			SMLocations.Emplace(FVector(FMath::FRandRange(50.f, 250.f), FMath::FRandRange(50.f, 70.f), FMath::FRandRange(30.f, 50.f)));
		}
		FLinearColor Col = FLinearColor::Green;
		Col.A = 0.3;
		UVizMarker* M = VizMarkerManager->CreateMarker(SMLocations, DoorSMA->GetStaticMeshComponent());
	}
#endif //SL_WITH_DATA_VIS
}

void USLDataVisualizer::MarkerTests()
{
#if SL_WITH_DATA_VIS
	TArray<FVector> Locations;
	for (uint32 Idx = 0; Idx < 1300; Idx++)
	{
		Locations.Emplace(FVector(FMath::FRandRange(50.f, 250.f), FMath::FRandRange(50.f, 70.f), FMath::FRandRange(30.f, 50.f)));
	}
	VizMarkerManager->CreateMarker(Locations);

	VizMarkerManager->CreateMarker(FVector(FMath::FRandRange(5.f, 25.f), FMath::FRandRange(5.f, 7.f), FMath::FRandRange(3.f, 5.f)),
		EVizMarkerType::Cylinder, FVector(0.2), FColor::Blue, true);

	//UOjh0Zb81U2QnDJCg2sOgA
	//zUDyz+WsLE6Onk4Nv5lhzg

	if (AStaticMeshActor* DoorSMA = FSLEntitiesManager::GetInstance()->GetStaticMeshActor("zUDyz+WsLE6Onk4Nv5lhzg"))
	{
		TArray<FVector> SMLocations;
		for (uint32 Idx = 0; Idx < 10; Idx++)
		{
			SMLocations.Emplace(FVector(FMath::FRandRange(50.f, 250.f), FMath::FRandRange(50.f, 70.f), FMath::FRandRange(30.f, 50.f)));
		}
		FLinearColor Col = FLinearColor::Green;
		Col.A = 0.3;
		UVizMarker* M = VizMarkerManager->CreateMarker(SMLocations, DoorSMA->GetStaticMeshComponent());
		M->Init(EVizMarkerType::Arrow, FVector(0.2), Col, false);
		M->Add(SMLocations);
		UVizHighlightMarker* DoorHM =  VizMarkerManager->CreateHighlightMarker(DoorSMA->GetStaticMeshComponent(), Col, EVizHighlightMarkerType::Translucent);




		if (AStaticMeshActor* SMA = FSLEntitiesManager::GetInstance()->GetStaticMeshActor("UOjh0Zb81U2QnDJCg2sOgA"))
		{
			TArray<FVector> SMLocations2;
			for (uint32 Idx = 0; Idx < 10; Idx++)
			{
				SMLocations2.Emplace(FVector(FMath::FRandRange(150.f, 1250.f), FMath::FRandRange(150.f, 170.f), FMath::FRandRange(130.f, 150.f)));
			}
			FLinearColor Col5 = FLinearColor::Red;
			Col5.A = 0.3;
			VizMarkerManager->CreateMarker(SMLocations2, SMA->GetStaticMeshComponent(), false, FLinearColor::Red);
			VizMarkerManager->CreateHighlightMarker(SMA->GetStaticMeshComponent(), Col5, EVizHighlightMarkerType::Additive);

			DoorHM->Init(SMA->GetStaticMeshComponent());
		}
	}



	// Left hand
	if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor("Qq1TK5TJnUuJlBQ7KF4eXA"))
	{
		FLinearColor Col = FLinearColor::Blue;
		Col.A = 0.3;
		VizMarkerManager->CreateHighlightMarker(SkMA->GetSkeletalMeshComponent(), 2, Col, EVizHighlightMarkerType::Additive);
	}

	// Right hand
	if (ASkeletalMeshActor* SkRightHandMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor("qKTna1rpDEWzLmHNrWtxtw"))
	{
		FLinearColor Col = FLinearColor::Yellow;
		Col.A = 0.3;
		UVizHighlightMarker* RightHamd = VizMarkerManager->CreateHighlightMarker(SkRightHandMA->GetSkeletalMeshComponent(), Col, EVizHighlightMarkerType::Translucent);

		// Adam
		if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor("NK9/T/b9lEWG/rVl6TIIDQ"))
		{
			FLinearColor Col2 = FLinearColor::Gray;
			Col2.A = 0.3;
			//UVizHighlightMarker* HM = VizMarkerManager->CreateHighlightMarker(SkMA->GetSkeletalMeshComponent(), 0, Col2, EVizHighlightMarkerType::Translucent);

			//HM->Init(SkMA->GetSkeletalMeshComponent(), 1, Col2);
			RightHamd->Init(SkMA->GetSkeletalMeshComponent(), 0, Col2);
		}
	}

	//uint32 MarkerId = MarkerManager->CreateMarkerArray(Locations);
	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Marker %ld created.."), *FString(__func__), __LINE__, MarkerId);
#endif //SL_WITH_DATA_VIS
}