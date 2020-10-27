// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Knowrob/SLKRWSClient.h"
#include "Knowrob/SLKREventDispatcher.h"
#include "Viz/SLVizStructs.h"
#include "VizQ/SLVizQBase.h"
#include "SLKnowrobManager.generated.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;


/**
 * Highlight hack
 */
USTRUCT()
struct FSLVizHighlightHackStruct
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString IndividualId = TEXT("DefaultIndividualId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;
};

/**
 * Marker test hack struct
 */
USTRUCT()
struct FSLVizMarkerHackStruct
{
	GENERATED_BODY();

	/* Query data */
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString IndividualId = TEXT("DefaultIndividualId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bIsSkeletal = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float EndTime = -1.f;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	float DeltaT = -1.f;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MarkerId = TEXT("DefaultMarkerId");


	/* Primitive marker */
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Primitive")
	bool bUsePrimitiveMesh = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Primitive", meta = (editcondition = "bUsePrimitiveMesh"))
	ESLVizPrimitiveMarkerType PrimitiveType = ESLVizPrimitiveMarkerType::Box;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Primitive", meta = (editcondition = "bUsePrimitiveMesh"))
	float Size = .1f;


	/* Custom color */
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Color")
	bool bUseCustomColor = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Color", meta = (editcondition = "bUseCustomColor"))
	FLinearColor Color = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Color", meta = (editcondition = "bUseCustomColor"))
	ESLVizMaterialType MaterialType = ESLVizMaterialType::Unlit;


	/* Timeline */
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Timeline")
	bool bAsTimeline = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Timeline", meta = (editcondition = "bAsTimeline"))
	float Duration = -1.f;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Timeline", meta = (editcondition = "bAsTimeline"))
	bool bLoop = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Timeline", meta = (editcondition = "bAsTimeline"))
	float UpdateRate = -1.f;
};



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

	// Auto connect to mongodb at init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Knowrob")
	bool bAutoConnectToKnowrob = true;

	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	int32 MongoServerPort = 27017;

	// Auto connect to mongodb at init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Mongo")
	bool bAutoConnectToMongo = true;

	// Websocket connection to knowrob
	TSharedPtr<FSLKRWSClient> KRWSClient;

	// Handle the protobuf message
	TSharedPtr<FSLKREventDispatcher> KREventDispatcher;

	/* Managers */
	// Manages the mongo connection
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLMongoQueryManager* MongoQueryManager;

	// Manages the visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizManager* VizManager;

	// Auto init world to viz
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	bool bAutoConvertWorld = true;


	/****************************************************************/
	/*							VizQ								*/
	/****************************************************************/
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	TArray<USLVizQBase*> Queries;

	/****************************************************************/
	/*					Editor button hacks							*/
	/****************************************************************/
	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bInitButtonHack = false;

	// Triggers a call to init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bStartButtonHack = false;

	// Triggers a call to init or reset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Setup Buttons")
	bool bFinishButtonHack = false;


	/****************************************************************/
	/*						Episode data							*/
	/****************************************************************/
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	bool bMongoConnectButtonHack = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	bool bMongoDisconnectButtonHack = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	FString TaskIdValueHack = TEXT("DefaultTaskId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	bool bSetTaskButtonhack = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	FString EpisodeIdValueHack = TEXT("DefaultEpisodeId");

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Episode Buttons")
	bool bSetEpisodeButtonHack = false;


	/****************************************************************/
	/*						VIZ episode replay						*/
	/****************************************************************/
	// Triggers a call to set the world as visual only
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bSetupWorldForEpisodeReplayButtonHack = false;

	// Triggers an episode query call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bLoadEpisodeDataButtonHack = false;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	float GotoValueHack = 0.f;

	// Triggers a goto call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bGotoButtonHack = false;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	float ReplayBeginValueHack = -1.f;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	float ReplayEndValueHack = -1.f;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bReplayLoopValueHack = true;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	float ReplayUpdateRateValueHack = -1.f;

	// Task to query from
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	FString ReplayTargetViewIdValueHack = TEXT("");

	// Triggers a goto call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bReplayButtonHack = false;

	// Triggers a pause call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bReplayPauseButtonHack = false;

	// Triggers a stop call
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Episode Buttons")
	bool bReplayStopButtonHack = false;


	/****************************************************************/
	/*						VIZ highlights							*/
	/****************************************************************/
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Highlight Buttons")
	bool bDrawSelectedHighlights = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Highlight Buttons")
	bool bUpdateHighlights = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Highlight Buttons")
	bool bRemoveAllHighlights = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Highlight Buttons")
	TArray<FSLVizHighlightHackStruct> HighlightValuesHack;


	/****************************************************************/
	/*						VIZ markers								*/
	/****************************************************************/
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Markers Buttons")
	bool bDrawMarkers = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Markers Buttons")
	bool bRemoveAllMarkers = false;

	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Viz Markers Buttons")
	TArray<FSLVizMarkerHackStruct> MarkerValuesHack;



	/****************************************************************/
	/*						Mongo query tests						*/
	/****************************************************************/
	/* MONGO Editor button hacks */
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
