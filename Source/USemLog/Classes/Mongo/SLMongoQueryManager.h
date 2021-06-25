// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Mongo/SLMongoQueryDBHandler.h"
#include "SLMongoQueryManager.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Mongo Query Manager")
class ASLMongoQueryManager : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLMongoQueryManager();

//#if WITH_EDITOR
//	// Called when a property is changed in the editor
//	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
//#endif // WITH_EDITOR

public:
	// Connect to the server
	bool Connect(const FString& ServerIp, uint16 ServerPort);

	// Disconnect from server
	void Disconnect();

	// Set the active task (database in mongo)
	bool SetTask(const FString& InTaskId);

	// Set the active episode (collection in mongo)
	bool SetEpisode(const FString& InEpisodeId);

	// Get init state
	bool IsConnected() const { return bConnected; };

	// Check if the task is selected
	bool IsTaskSet() const { return bTaskSet; };

	// Check if the episode is selected
	bool IsEpisodeSet() const { return bEpisodeSet; };

	/* Queries */
	// Get the individual pose
	FTransform GetIndividualPoseAt(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float Ts);
	FTransform GetIndividualPoseAt(const FString& InEpisodeId, const FString& IndividualId, float Ts);
	FTransform GetIndividualPoseAt(const FString& IndividualId, float Ts) const;

	// Get the individual trajectory
	TArray<FTransform> GetIndividualTrajectory(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f);
	TArray<FTransform> GetIndividualTrajectory(const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f);
	TArray<FTransform> GetIndividualTrajectory(const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f) const;

	// Get skeletal individual pose
	TPair<FTransform, TMap<int32, FTransform>> GetSkeletalIndividualPoseAt(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float Ts);
	TPair<FTransform, TMap<int32, FTransform>> GetSkeletalIndividualPoseAt(const FString& InEpisodeId, const FString& IndividualId, float Ts);
	TPair<FTransform, TMap<int32, FTransform>> GetSkeletalIndividualPoseAt(const FString& IndividualId, float Ts) const;

	// Get skeletal individual trajectory
	TArray<TPair<FTransform, TMap<int32, FTransform>>>  GetSkeletalIndividualTrajectory(const FString& InTaskId, const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f);
	TArray<TPair<FTransform, TMap<int32, FTransform>>>  GetSkeletalIndividualTrajectory(const FString& InEpisodeId, const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f);
	TArray<TPair<FTransform, TMap<int32, FTransform>>>  GetSkeletalIndividualTrajectory(const FString& IndividualId, float StartTs, float EndTs, float DeltaT = -1.f) const;

	// Get the episode data
	TArray<TPair<float, TMap<FString, FTransform>>> GetEpisodeData(const FString& InTaskId, const FString& InEpisodeId);
	TArray<TPair<float, TMap<FString, FTransform>>> GetEpisodeData(const FString& InEpisodeId);
	TArray<TPair<float, TMap<FString, FTransform>>> GetEpisodeData() const;

	// Spawn or get manager from the world
	static ASLMongoQueryManager* GetExistingOrSpawnNew(UWorld* World);

protected:
	// True when successfully connected to the server
	bool bConnected : 1;

	// Task set to query from
	bool bTaskSet : 1;

	// Episode set to query from
	bool bEpisodeSet : 1;

private:
	// Current active task
	FString TaskId;

	// Current active episode
	FString EpisodeId;

	// Database handler
	FSLMongoQueryDBHandler DBHandler;

	///* Editor button hacks */
	//// Server ip to connect to
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//FString ServerIpValueHack = TEXT("127.0.0.1");
	//
	//// Server port to connect to
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//uint16 ServerPortValueHack = 27017;

	//// Triggers a call to connect
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//bool bMongoConnectButtonHack = false;

	//// Triggers a call to discconnect
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//bool bMongoDisconnectButtonHack = false;

	//// Task to query from
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//FString TaskIdValueHack = TEXT("DefaultTaskId");

	//// Triggers a call to set task
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//bool bSetTaskButtonhack = false;

	//// Episode to query from
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//FString EpisodeIdValueHack = TEXT("DefaultEpisodeId");

	//// Triggers a call to set episode
	//UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Buttons")
	//bool bSetEpisodeButtonHack = false;

	//// Individual id to query
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//FString IndividualIdValueHack = TEXT("DefaultIndividualId");

	//// Time to query for (pose or trajectories)
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//float StartTimestampValueHack = 0.f;

	//// Triggers a pose query call
	//UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Buttons")
	//bool bPoseQueryButtonHack = false;

	//// End time to query for (trajectories only)
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//float EndTimestampValueHack = 10.f;

	//// Trajectory delta time
	//UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	//float DeltaTValueHack = -1.f;

	//// Triggers a trajectory query call
	//UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Buttons")
	//bool bTrajectoryQueryButtonHack = false;

	//// Triggers an episode query call
	//UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Buttons")
	//bool bLoadEpisodeDataButtonHack = false;

	//// Triggers a trajectory query call
	//UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Buttons")
	//bool bSkelTrajectoryQueryButtonHack = false;
};
