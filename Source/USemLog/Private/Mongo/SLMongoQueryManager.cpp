// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Mongo/SLMongoQueryManager.h"
#include "EngineUtils.h"

// Ctor
ASLMongoQueryManager::ASLMongoQueryManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	bConnected = false;
	bTaskSet = false;
	bEpisodeSet = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

//#if WITH_EDITOR
//// Called when a property is changed in the editor
//void ASLMongoQueryManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeProperty(PropertyChangedEvent);
//
//	// Get the changed property name
//	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
//		PropertyChangedEvent.Property->GetFName() : NAME_None;
//
//	/* Button hacks */
//	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bMongoConnectButtonHack))
//	{
//		bMongoConnectButtonHack = false;
//		if (Connect(ServerIpValueHack, ServerPortValueHack))
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Connected to %s:%ld"), *FString(__FUNCTION__), __LINE__, *ServerIpValueHack, ServerPortValueHack);
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to connect to %s:%ld"), *FString(__FUNCTION__), __LINE__, *ServerIpValueHack, ServerPortValueHack);
//		}
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bMongoDisconnectButtonHack))
//	{
//		bMongoDisconnectButtonHack = false;
//		Disconnect();
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d Disconnected.."), *FString(__FUNCTION__), __LINE__);
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bSetTaskButtonhack))
//	{
//		bSetTaskButtonhack = false;
//		if (SetTask(TaskIdValueHack))
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Set task to %s"), *FString(__FUNCTION__), __LINE__, *TaskIdValueHack);
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to set task to %s"), *FString(__FUNCTION__), __LINE__, *TaskIdValueHack);
//		}
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bSetEpisodeButtonHack))
//	{
//		bSetEpisodeButtonHack = false;
//		if (SetEpisode(EpisodeIdValueHack))
//		{
//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Set episode to %s"), *FString(__FUNCTION__), __LINE__, *EpisodeIdValueHack);
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to set episode to %s"), *FString(__FUNCTION__), __LINE__, *EpisodeIdValueHack);
//		}
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bPoseQueryButtonHack))
//	{
//		bPoseQueryButtonHack = false;
//
//		FTransform Pose = GetIndividualPoseAt(TaskIdValueHack, EpisodeIdValueHack, 
//			IndividualIdValueHack, StartTimestampValueHack);
//
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] IndividualPose: Loc=%s; \t Quat=%s; \t (%s:%s:%s)"),
//			*FString(__FUNCTION__), __LINE__, StartTimestampValueHack, *Pose.GetLocation().ToString(), *Pose.GetRotation().ToString(),
//			*TaskIdValueHack, *EpisodeIdValueHack, *IndividualIdValueHack);
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bTrajectoryQueryButtonHack))
//	{
//		bTrajectoryQueryButtonHack = false;
//		TArray<FTransform> Trajectory = GetIndividualTrajectory(TaskIdValueHack, EpisodeIdValueHack,
//			IndividualIdValueHack, StartTimestampValueHack, EndTimestampValueHack, DeltaTValueHack);
//
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f-%f] IndividualTrajectoryNum=%ld; (%s:%s:%s)"),
//			*FString(__FUNCTION__), __LINE__, StartTimestampValueHack, EndTimestampValueHack, Trajectory.Num(),
//			*TaskIdValueHack, *EpisodeIdValueHack, *IndividualIdValueHack);
//
//		for (const auto& Pose : Trajectory)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("\t\t\t\t Loc=%s; \t Quat=%s;"), *Pose.GetLocation().ToString(), *Pose.GetRotation().ToString());
//		}
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bLoadEpisodeDataButtonHack))
//	{
//		bLoadEpisodeDataButtonHack = false;
//		TArray<TPair<float, TMap<FString, FTransform>>> EpisodeData;
//		GetEpisodeData(TaskIdValueHack, EpisodeIdValueHack,	EpisodeData);
//
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d EpisodeDataNum=%ld; (%s:%s)"),
//			*FString(__FUNCTION__), __LINE__, EpisodeData.Num(), *TaskIdValueHack, *EpisodeIdValueHack);
//
//		for (const auto& Frame : EpisodeData)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("\t\t\t Frame: Ts=%f; \t IndividualsNum=%d;"), Frame.Key, Frame.Value.Num());
//		}
//	}
//	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLMongoQueryManager, bSkelTrajectoryQueryButtonHack))
//	{
//		bTrajectoryQueryButtonHack = false;
//		TArray<TPair<FTransform, TMap<int32, FTransform>>> Trajectory = GetSkeletalIndividualTrajectory(TaskIdValueHack, EpisodeIdValueHack,
//			IndividualIdValueHack, StartTimestampValueHack, EndTimestampValueHack, DeltaTValueHack);
//
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f-%f] IndividualTrajectoryNum=%ld; (%s:%s:%s)"),
//			*FString(__FUNCTION__), __LINE__, StartTimestampValueHack, EndTimestampValueHack, Trajectory.Num(),
//			*TaskIdValueHack, *EpisodeIdValueHack, *IndividualIdValueHack);
//	}
//}
//#endif // WITH_EDITOR

// Connect to the server
bool ASLMongoQueryManager::Connect(const FString& ServerIp, uint16 ServerPort)
{
	if (bConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Already connected.. for choosing differet server, disconnect first.."),
			*FString(__FUNCTION__), __LINE__);
		return true;
	}
	if (DBHandler.Connect(ServerIp, ServerPort))
	{
		bConnected = true;
	}
	else
	{
		bConnected = false;
	}
	return bConnected;
}

// Disconnect from server
void ASLMongoQueryManager::Disconnect()
{
	if (bConnected)
	{
		DBHandler.Disconnect();
		TaskId = "";
		EpisodeId = "";
		
		bConnected = false;
		bTaskSet = false;
		bEpisodeSet = false;
	}
}

// Set the active task (database in mongo)
bool ASLMongoQueryManager::SetTask(const FString& InTaskId)
{
	if (!bConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Connect to server first.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	if (bTaskSet && TaskId.Equals(InTaskId))
	{
		return true;
	}
	if (DBHandler.SetDatabase(InTaskId))
	{
		TaskId = InTaskId;
		bTaskSet = true;	
	}
	else
	{
		TaskId = "";
		bTaskSet = false;
	}
	return bTaskSet;

}

// Set the active episode (collection in mongo)
bool ASLMongoQueryManager::SetEpisode(const FString& InEpisodeId)
{
	if (!bConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Connect to server first.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	if (!bTaskSet)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Set task first.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	if (bEpisodeSet && EpisodeId.Equals(InEpisodeId))
	{
		return true;
	}
	if (DBHandler.SetCollection(InEpisodeId))
	{
		EpisodeId = InEpisodeId;
		bEpisodeSet = true;
	}
	else
	{
		EpisodeId = "";
		bEpisodeSet = false;
	}
	return bEpisodeSet;
}

/* Queries */
// Get the individual pose with task and episode init
FTransform ASLMongoQueryManager::GetIndividualPoseAt(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float Ts)
{
	if (SetTask(InTaskId))
	{
		return GetIndividualPoseAt(InEpisodeId, IndividualId, Ts);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set task: %s .."), *FString(__FUNCTION__), __LINE__, *InTaskId);
		return FTransform();
	}
}

// Get the individual pose with episode init
FTransform ASLMongoQueryManager::GetIndividualPoseAt(const FString& InEpisodeId, const FString& IndividualId, float Ts)
{
	if (SetEpisode(InEpisodeId))
	{
		return GetIndividualPoseAt(IndividualId, Ts);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set episode: %s .."), *FString(__FUNCTION__), __LINE__, *InEpisodeId);
		return FTransform();
	}
}

// Get the individual pose
FTransform ASLMongoQueryManager::GetIndividualPoseAt(const FString& IndividualId, float Ts) const
{
	return DBHandler.GetIndividualPoseAt(IndividualId, Ts);
}

// Get the individual trajectory with task and episode init
TArray<FTransform> ASLMongoQueryManager::GetIndividualTrajectory(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT)
{
	if (SetTask(InTaskId))
	{
		return GetIndividualTrajectory(InEpisodeId, IndividualId, StartTs, EndTs, DeltaT);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set task: %s .."), *FString(__FUNCTION__), __LINE__, *InTaskId);
		return TArray<FTransform>();
	}
}

// Get the individual trajectory with episode init
TArray<FTransform> ASLMongoQueryManager::GetIndividualTrajectory(const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT)
{
	if (SetEpisode(InEpisodeId))
	{
		return GetIndividualTrajectory(IndividualId, StartTs, EndTs, DeltaT);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set episode: %s .."), *FString(__FUNCTION__), __LINE__, *InEpisodeId);
		return TArray<FTransform>();
	}
}

// Get the individual trajectory 
TArray<FTransform> ASLMongoQueryManager::GetIndividualTrajectory(const FString& IndividualId, float StartTs, float EndTs, float DeltaT) const
{
	return DBHandler.GetIndividualTrajectory(IndividualId, StartTs, EndTs, DeltaT);
}


// Get skeletal individual pose with task and episode init
TPair<FTransform, TMap<int32, FTransform>> ASLMongoQueryManager::GetSkeletalIndividualPoseAt(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float Ts)
{
	if (SetTask(InTaskId))
	{
		return GetSkeletalIndividualPoseAt(InEpisodeId, IndividualId, Ts);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set task: %s .."), *FString(__FUNCTION__), __LINE__, *InTaskId);
		return TPair<FTransform, TMap<int32, FTransform>>();
	}
}

// Get skeletal individual pose with episode init
TPair<FTransform, TMap<int32, FTransform>> ASLMongoQueryManager::GetSkeletalIndividualPoseAt(const FString& InEpisodeId, const FString& IndividualId, float Ts)
{
	if (SetEpisode(InEpisodeId))
	{
		return GetSkeletalIndividualPoseAt(IndividualId, Ts);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set episode: %s .."), *FString(__FUNCTION__), __LINE__, *InEpisodeId);
		return TPair<FTransform, TMap<int32, FTransform>>();
	}
}

// Get skeletal individual pose
TPair<FTransform, TMap<int32, FTransform>> ASLMongoQueryManager::GetSkeletalIndividualPoseAt(const FString& IndividualId, float Ts) const
{
	return DBHandler.GetSkeletalIndividualPoseAt(IndividualId, Ts);	
}

// Get skeletal individual trajectory with task and episode init
TArray<TPair<FTransform, TMap<int32, FTransform>>> ASLMongoQueryManager::GetSkeletalIndividualTrajectory(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT)
{
	if (SetTask(InTaskId))
	{
		return GetSkeletalIndividualTrajectory(InEpisodeId, IndividualId, StartTs, EndTs, DeltaT);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set task: %s .."), *FString(__FUNCTION__), __LINE__, *InTaskId);
		return TArray<TPair<FTransform, TMap<int32, FTransform>>>();
	}

}

// Get skeletal individual trajectoru with episode init
TArray<TPair<FTransform, TMap<int32, FTransform>>> ASLMongoQueryManager::GetSkeletalIndividualTrajectory(const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT)
{
	if (SetEpisode(InEpisodeId))
	{
		return GetSkeletalIndividualTrajectory(IndividualId, StartTs, EndTs, DeltaT);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set episode: %s .."), *FString(__FUNCTION__), __LINE__, *InEpisodeId);		
		return TArray<TPair<FTransform, TMap<int32, FTransform>>>();
	}
}

// Get skeletal individual trajectory
TArray<TPair<FTransform, TMap<int32, FTransform>>> ASLMongoQueryManager::GetSkeletalIndividualTrajectory(const FString& IndividualId, float StartTs, float EndTs, float DeltaT) const
{
	return DBHandler.GetSkeletalIndividualTrajectory(IndividualId, StartTs, EndTs, DeltaT);
}

// Get the episode data with task and episode init
TArray<TPair<float, TMap<FString, FTransform>>> ASLMongoQueryManager::GetEpisodeData(const FString& InTaskId, const FString& InEpisodeId)
{
	if (SetTask(InTaskId))
	{
		return GetEpisodeData(InEpisodeId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set task: %s .."), *FString(__FUNCTION__), __LINE__, *InTaskId);
		return TArray<TPair<float, TMap<FString, FTransform>>>();
	}
}

// Get the episode data with episode init
TArray<TPair<float, TMap<FString, FTransform>>> ASLMongoQueryManager::GetEpisodeData(const FString& InEpisodeId)
{
	if (SetEpisode(InEpisodeId))
	{
		return GetEpisodeData();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not set episode: %s .."), *FString(__FUNCTION__), __LINE__, *InEpisodeId);
		return TArray<TPair<float, TMap<FString, FTransform>>>();
	}
}

// Get the episode data
TArray<TPair<float, TMap<FString, FTransform>>> ASLMongoQueryManager::GetEpisodeData() const
{
	return DBHandler.GetEpisodeData();
}

// Spawn or get manager from the world
ASLMongoQueryManager* ASLMongoQueryManager::GetExistingOrSpawnNew(UWorld* World)
{
	// Check in world
	for (TActorIterator<ASLMongoQueryManager>Iter(World); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			return *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_MongoQueryManager");
	auto Manager = World->SpawnActor<ASLMongoQueryManager>(SpawnParams);
#if WITH_EDITOR
	Manager->SetActorLabel(TEXT("SL_MongoQueryManager"));
#endif // WITH_EDITOR
	return Manager;
}
