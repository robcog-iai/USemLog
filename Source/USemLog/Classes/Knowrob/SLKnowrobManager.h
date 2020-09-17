// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Knowrob/SLKRWSClient.h"
#include "SLKnowrobManager.generated.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;

/**
*
**/
UCLASS()
class USEMLOG_API ASLKnowrobManager : public AInfo
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ASLKnowrobManager();

	// Dtor
	~ASLKnowrobManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set up any required references and connect to server
	void Init();

	// Start processing any incomming messages
	void Start();

	// Stop processing the messages, and disconnect from server
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	/* Knowrob websocket client delegate triggers */
	// Called when a new message is received from knowrob
	void OnKRMsg();

	// Called when connected or disconnecetd with knowrob
	void OnKRConnection(bool bConnectionValue);

private:
	// Get the mongo query manager from the world (or spawn a new one)
	bool SetMongoQueryManager();

	// Get the viz manager from the world (or spawn a new one)
	bool SetVizManager();

protected:
	// True when all references are set and it is connected to the server
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True when active
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsStarted : 1;

	// True when done logging
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsFinished : 1;

private:
	// Knowrob server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	FString KRServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	int32 KRServerPort = 8080;
	
	// Websocket protocal
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	FString KRWSProtocol = TEXT("prolog_websocket");

	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	int32 MongoServerPort = 27017;

	// Websocket connection to knowrob
	TSharedPtr<FSLKRWSClient> KRWSClient;

	/* Managers */
	// Manages the mongo connection
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLMongoQueryManager* MongoQueryManager;

	// Manages the visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizManager* VizManager;	


	/* Editor button hacks */
	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bInitButtonHack = false;

	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bStartButtonHack = false;

	// Triggers a call to init or reset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bFinishButtonHack = false;


	/* MONGO Editor button hacks */
	// Triggers a call to set the world as visual only
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Buttons")
	bool bSetupWorldForEpisodeReplayButtonHack = false;

	// Triggers an episode query call
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Viz Buttons")
	bool bLoadEpisodeDataButtonHack = false;



	/* MONGO Editor button hacks */
	// Triggers a call to connect
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	bool bMongoConnectButtonHack = false;

	// Triggers a call to discconnect
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	bool bMongoDisconnectButtonHack = false;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	FString TaskIdValueHack = TEXT("DefaultTaskId");

	// Triggers a call to set task
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	bool bSetTaskButtonhack = false;

	// Episode to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	FString EpisodeIdValueHack = TEXT("DefaultEpisodeId");

	// Triggers a call to set episode
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Mongo Buttons")
	bool bSetEpisodeButtonHack = false;

	// Individual id to query
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	FString IndividualIdValueHack = TEXT("DefaultIndividualId");

	// Time to query for (pose or trajectories)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	float StartTimestampValueHack = 0.f;

	// Triggers a pose query call
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Mongo Buttons")
	bool bPoseQueryButtonHack = false;

	// End time to query for (trajectories only)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	float EndTimestampValueHack = 10.f;

	// Trajectory delta time
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo Buttons")
	float DeltaTValueHack = -1.f;

	// Triggers a trajectory query call
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Mongo Buttons")
	bool bTrajectoryQueryButtonHack = false;


};
