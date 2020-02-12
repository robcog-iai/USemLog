// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLDataVisualizer.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

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
		//MarkerManager = NewObject<AVizMarkerManager>(this);
		//MarkerManager->Init();
		//if (!MarkerManager)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not create the marker manager.."), *FString(__func__), __LINE__);
		//	return;
		//}
		//MarkerManager->RegisterComponent();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("SL_VizMarkerManager");		
		MarkerManager = GetWorld()->SpawnActor<AVizMarkerManager>(AVizMarkerManager::StaticClass(),
			FTransform(FRotator::ZeroRotator, FVector::ZeroVector), SpawnParams);
		if (!MarkerManager)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not spawn the MarkerManager.."), *FString(__func__), __LINE__);
			return;
		}
		MarkerManager->SetActorLabel(TEXT("SL_VizMarkerManager"));
		MarkerManager->Init();

		if (!QAHandler.ConnectToServer(VisQueries->ServerIP, VisQueries->ServerPort))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Query handler not connect to server.."), *FString(__func__), __LINE__);
			return;
		}
		
		bIsInit = true;		
#endif //SL_WITH_DATA_VIS
	}
}

// Start logger
void USLDataVisualizer::Start(const FName& UserInputActionName)
{
	if (!bIsStarted && bIsInit)
	{
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
			IC->BindAction(UserInputActionName, IE_Released, this, &USLDataVisualizer::Visualize);
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
void USLDataVisualizer::Visualize()
{
	
#if SL_WITH_DATA_VIS	
	QueryIdx++;

	uint32 MarkerId = MarkerManager->CreateMarker(FVector(0), EVizMarkerType::Sphere, FVector(2), FLinearColor::Red, true);
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Marker %ld created.."), *FString(__func__), __LINE__, MarkerId);

	if (VisQueries->Queries.IsValidIndex(QueryIdx))
	{
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
			case ESLVisQueryType::EntityPose:
			{
				FTransform Pose;
				QAHandler.GetEntityPoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					Pose);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] EntityPose result:\t T=[%s];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, *Pose.ToString());
				break;
			}

			case ESLVisQueryType::EntityTraj:
			{
				TArray<FTransform> Traj;
				QAHandler.GetEntityTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					Traj);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] EntityTraj result:\t TrajNum=[%ld];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, Traj.Num());
				//for (const auto& T : Traj)
				//{
				//	UE_LOG(LogTemp, Warning, TEXT(" \t\t\t %s"), *T.ToString());
				//}				
				break;
			}

			case ESLVisQueryType::BonePose:
			{
				FTransform Pose;
				QAHandler.GetBonePoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].BoneName,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					Pose);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] BonePose result:\t T=[%s];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, *Pose.ToString());
				break;
			}

			case ESLVisQueryType::BoneTraj:
			{
				TArray<FTransform> Traj;
				QAHandler.GetBoneTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].BoneName,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					Traj);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] BoneTraj result: TrajNum=[%ld];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, Traj.Num());
				//for (const auto& T : Traj)
				//{
				//	UE_LOG(LogTemp, Log, TEXT(" \t\t\t %s"), *T.ToString());
				//}
				break;
			}

			case ESLVisQueryType::SkeletalPose:
			{
				TPair<FTransform, TMap<FString, FTransform>> SkeletalPose;
				QAHandler.GetSkeletalPoseAt(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					SkeletalPose);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] SkeletalPose result: T=[%s]; BoneNum=[%ld];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx,
					*SkeletalPose.Key.ToString(), SkeletalPose.Value.Num());
				for (const auto& Pair : SkeletalPose.Value)
				{
					UE_LOG(LogTemp, Log, TEXT(" \t\t\t %s-%s"), *Pair.Key, *Pair.Value.ToString());
				}
				break;
			}

			case ESLVisQueryType::SkeletalTraj:
			{
				TArray<TPair<FTransform, TMap<FString, FTransform>>> SkeletalTraj;
				QAHandler.GetSkeletalTrajectory(VisQueries->Queries[QueryIdx].EntityId,
					VisQueries->Queries[QueryIdx].StartTimestamp,
					VisQueries->Queries[QueryIdx].EndTimestamp,
					SkeletalTraj);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] SkeletalTraj result: TrajNum=[%ld];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, SkeletalTraj.Num());
				break;
			}

			case ESLVisQueryType::WorldState:
			{
				TMap<FString, FTransform> Entities;
				TMap<FString, TPair<FTransform, TMap<FString, FTransform>>> Skeletals;
				QAHandler.GetWorldStateAt(VisQueries->Queries[QueryIdx].StartTimestamp,
					Entities, Skeletals);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] WorldState result: EntitiesNum=[%ld]; SkeletalsNum=[%d]"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, Entities.Num(), Skeletals.Num());
				break;
			}

			case ESLVisQueryType::Episode:
			{
				TArray<FMQWorldStateFrame> WorldStates;
				QAHandler.GetEpisode(VisQueries->Queries[QueryIdx].StartTimestamp, VisQueries->Queries[QueryIdx].EndTimestamp,
					WorldStates);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Q[%ld] Episode result: WorldStatesNum=[%ld];"),
					*FString(__FUNCTION__), __LINE__, QueryIdx, WorldStates.Num());
				break;
			}

			default:
				UE_LOG(LogTemp, Error, TEXT("%s::%d Wrong query type.."), *FString(__FUNCTION__), __LINE__);				
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No more queries.."), *FString(__func__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}
