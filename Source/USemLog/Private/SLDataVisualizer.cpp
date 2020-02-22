// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLDataVisualizer.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#include "SLEntitiesManager.h"

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
		if (!InQueries)
		{
			return;
		}
		VisQueries = InQueries;

#if SL_WITH_DATA_VIS
		/* Check connection to database */
		if (!QAHandler.ConnectToServer(VisQueries->ServerIP, VisQueries->ServerPort))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Query handler not connect to server.."), *FString(__func__), __LINE__);
			return;
		}


		/* Spawn marker viz manager */
		FActorSpawnParameters VizMarkerSpawnParams;
		VizMarkerSpawnParams.Name = TEXT("SL_VizMarkerManager");		
		VizMarkerManager = GetWorld()->SpawnActor<AVizMarkerManager>(AVizMarkerManager::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector::ZeroVector), VizMarkerSpawnParams);
		if (!VizMarkerManager)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn the MarkerManager.."), *FString(__func__), __LINE__);
			return;
		}
		VizMarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));

		/* Spawn world viz manager */
		FActorSpawnParameters VizWorldSpawnParams;
		VizWorldSpawnParams.Name = TEXT("SL_VizWorldManager");
		VizWorldManager = GetWorld()->SpawnActor<AVizWorldManager>(AVizWorldManager::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector::ZeroVector), VizWorldSpawnParams);
		if (!VizWorldManager)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn the MarkerManager.."), *FString(__func__), __LINE__);
			return;
		}
		VizWorldManager->SetActorLabel(TEXT("SL_VizWorldManager"));
		
		
		bIsInit = true;		
#endif //SL_WITH_DATA_VIS
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
			
			VizWorldManager->Init(SkeletalActors);
			
			// Get the episode data from mongo
			TArray<FMQWorldStateFrame> Episode;
			QAHandler.GetAllWorldStates(Episode);

			// Set up episode in the world manager
			for (const auto Frame : Episode)
			{
				//TODO
				// Re-create map using actor pointer
				TMap<AStaticMeshActor*, FTransform> SMAPoses;
				TMap<ASkeletalMeshActor*, TPair<FTransform, TMap<FString, FTransform>>> SkMAPoses;


				VizWorldManager->AddFrame(Frame.Timestamp, SMAPoses, SkMAPoses);
			}
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
			QAHandler.SetDatabase(DBName);
			PrevDBName = DBName;
		}

		const FString CollName = VisQueries->Queries[QueryIdx].EpisodeId;
		if (!PrevCollName.Equals(CollName))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting collection to %s.."), *FString(__func__), __LINE__, *CollName);
			QAHandler.SetCollection(CollName);
			PrevCollName = CollName;
		}

		/* Query */
		switch (VisQueries->Queries[QueryIdx].QueryType)
		{
			// Entity pose
			case ESLVisQueryType::EntityPose:
			{
				FTransform Pose;
				if (QAHandler.GetEntityPoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					Pose))
				{
					EntityPoseResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].StartTimestamp, Pose);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d EntityPose query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Entity traj
			case ESLVisQueryType::EntityTraj:
			{
				TArray<FTransform> Traj;
				if (QAHandler.GetEntityTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					Traj, 0.5f))
				{
					EntityTrajResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].StartTimestamp,
						VisQueries->Queries[QueryIdx].EndTimestamp, Traj);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d EntityTraj query failed.."), *FString(__FUNCTION__), __LINE__);
				}		
				break;
			}

			// Bone pose
			case ESLVisQueryType::BonePose:
			{
				FTransform Pose;
				if(QAHandler.GetBonePoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].BoneName,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					Pose))
				{
					BonePoseResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].BoneName,
						VisQueries->Queries[QueryIdx].StartTimestamp, Pose);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d BonePose query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Bone traj
			case ESLVisQueryType::BoneTraj:
			{
				TArray<FTransform> Traj;
				if (QAHandler.GetBoneTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].BoneName,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					Traj))
				{
					BoneTrajResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].BoneName,
						VisQueries->Queries[QueryIdx].StartTimestamp, VisQueries->Queries[QueryIdx].EndTimestamp, Traj);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d BoneTraj query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Skeletal pose
			case ESLVisQueryType::SkeletalPose:
			{
				TPair<FTransform, TMap<FString, FTransform>> SkeletalPose;
				if (QAHandler.GetSkeletalPoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					SkeletalPose))
				{
					SkelPoseResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].StartTimestamp, SkeletalPose);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d SkeletalPose query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Skeletal traj
			case ESLVisQueryType::SkeletalTraj:
			{
				TArray<TPair<FTransform, TMap<FString, FTransform>>> SkeletalTraj;
				if (QAHandler.GetSkeletalTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					SkeletalTraj))
				{
					SkelTrajResult(VisQueries->Queries[QueryIdx].EntityId, VisQueries->Queries[QueryIdx].StartTimestamp, VisQueries->Queries[QueryIdx].EndTimestamp,
						SkeletalTraj);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d SkeletalTraj query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Gaze pose
			case ESLVisQueryType::GazePose:
			{
				FVector Target;
				FVector Origin;
				if (QAHandler.GetGazePose(VisQueries->Queries[QueryIdx].StartTimestamp, Target, Origin))
				{
					GazePoseResult(VisQueries->Queries[QueryIdx].StartTimestamp, Target, Origin);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d GazePose query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// Gaze traj
			case ESLVisQueryType::GazeTraj:
			{
				TArray<FVector> Targets;
				TArray<FVector> Origins;
				if (QAHandler.GetGazeTrajectory(VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp, Targets, Origins))
				{
					GazeTrajResult(VisQueries->Queries[QueryIdx].StartTimestamp, VisQueries->Queries[QueryIdx].EndTimestamp, Targets, Origins);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d GazeTraj query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// World state
			case ESLVisQueryType::WorldState:
			{
				TMap<FString, FTransform> Entities;
				TMap<FString, TPair<FTransform, TMap<FString, FTransform>>> Skeletals;
				if (QAHandler.GetWorldStateAt(VisQueries->Queries[QueryIdx].StartTimestamp, Entities, Skeletals))
				{
					WorldStateResult(VisQueries->Queries[QueryIdx].StartTimestamp, Entities, Skeletals);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d WorldState query failed.."), *FString(__FUNCTION__), __LINE__);
				}
				break;
			}

			// All world states
			case ESLVisQueryType::AllWorldStates:
			{
				TArray<FMQWorldStateFrame> WorldStates;
				if (QAHandler.GetAllWorldStates(WorldStates))
				{
					AllWorldStatesResult(WorldStates);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d WorldState query failed.."), *FString(__FUNCTION__), __LINE__);
				}
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


/* Results */
void USLDataVisualizer::EntityPoseResult(const FString& Id, float Ts, const FTransform& Pose)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityPose \t [%s:%f] :\t T=[%s];"),
		*FString(__FUNCTION__), __LINE__, *Id, Ts, *Pose.ToString());
}

void USLDataVisualizer::EntityTrajResult(const FString& Id, float StartTime, float EndTime, const TArray<FTransform>& Traj)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityTraj \t [%s:%f-%f] :\t TNum=[%ld];"),
		*FString(__FUNCTION__), __LINE__, *Id, StartTime, EndTime, Traj.Num());
}

void USLDataVisualizer::BonePoseResult(const FString& Id, const FString& BoneName, float Ts, const FTransform& Pose)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d BonePose \t [%s-%s:%f] :\t T=[%s];"),
		*FString(__FUNCTION__), __LINE__, *Id, *BoneName, Ts, *Pose.ToString());
}

void USLDataVisualizer::BoneTrajResult(const FString& Id, const FString& BoneName, float StartTime, float EndTime, const TArray<FTransform>& Traj)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d BoneTraj \t [%s-%s:%f-%f] :\t TNum=[%ld];"),
		*FString(__FUNCTION__), __LINE__, *Id, *BoneName, StartTime, EndTime, Traj.Num());
}

void USLDataVisualizer::SkelPoseResult(const FString& Id, float Ts, const TPair<FTransform, TMap<FString, FTransform>>& SkelPose)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d SkeletalPose \t [%s:%f] :\t T=[%s]; TBoneNum=[%ld];"),
		*FString(__FUNCTION__), __LINE__, *Id, Ts, *SkelPose.Key.ToString(), SkelPose.Value.Num());

	//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor(Id))
	//{
	//	FLinearColor Col = FLinearColor::Blue;
	//	Col.A = 0.3;
	//	VizMarkerManager->CreateMarker(SkelPose, SkMA->GetSkeletalMeshComponent(), 2, false, Col);
	//	UE_LOG(LogTemp, Error, TEXT(" !!!! Skeletal pose created"));
	//}
}

void USLDataVisualizer::SkelTrajResult(const FString& Id, float StartTime, float EndTime, const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkelTraj)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d SkeletalTraj \t [%s:%f-%f] :\t TNum=[%ld];"),
		*FString(__FUNCTION__), __LINE__, *Id, StartTime, EndTime, SkelTraj.Num());

	//if (ASkeletalMeshActor* SkMA = FSLEntitiesManager::GetInstance()->GetSkeletalMeshActor(VisQueries->Queries[QueryIdx].EntityId))
	//{
	//	FLinearColor Col = FLinearColor::Red;
	//	Col.A = 0.3;
	//	VizMarkerManager->CreateMarker(SkeletalTraj, SkMA->GetSkeletalMeshComponent(), 5, false, Col);

	//	UE_LOG(LogTemp, Log, TEXT(" !!!! Skeletal Traj created"));
	//}
}

void USLDataVisualizer::GazePoseResult(float Ts, FVector Target, FVector Origin)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d GazePose \t [%f] :\t Target=[%s]; Origin=[%s]"),
		*FString(__FUNCTION__), __LINE__, Ts, *Target.ToString(), *Origin.ToString());
}

void USLDataVisualizer::GazeTrajResult(float StartTime, float EndTime, const TArray<FVector>& Targets, const TArray<FVector>& Origins)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d GazeTraj \t [%f-%f] :\t TargetNum=[%ld]; OriginNum=[%ld]"),
		*FString(__FUNCTION__), __LINE__, StartTime, EndTime, Targets.Num(), Origins.Num());
}

void USLDataVisualizer::WorldStateResult(float Ts, const TMap<FString, FTransform>& Entities, const TMap<FString, TPair<FTransform, TMap<FString, FTransform>>>& Skeletals)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d WorldState \t [%f] :\t EntitiesNum=[%ld]; SkelNum=[%ld]"),
		*FString(__FUNCTION__), __LINE__, Ts, Entities.Num(), Skeletals.Num());
}

void USLDataVisualizer::AllWorldStatesResult(const TArray<FMQWorldStateFrame>& WorldStates)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d AllWorldStates \t :\t StatesNum=[%ld];"),
		*FString(__FUNCTION__), __LINE__, WorldStates.Num());
}


// TESTS
void USLDataVisualizer::SMTest()
{
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
}

void USLDataVisualizer::MarkerTests()
{
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
}