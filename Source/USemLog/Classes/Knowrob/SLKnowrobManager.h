// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Knowrob/SLLevelManager.h"
#include "Knowrob/SLKRWSClient.h"
#include "Knowrob/SLKRMsgDispatcher.h"
#include "Viz/SLVizStructs.h"
#include "VizQ/SLVizQBase.h"
#include "Runtime/SLLoggerStructs.h"
#include "SLKnowrobManager.generated.h"

// Forward declarations
class ASLMongoQueryManager;
class ASLVizManager;
class ASLVizSemMapManager;
class USLVizQBase;
class ASLControlManager;
class ASLSymbolicLogger;
class ASLWorldStateLogger;

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

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
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

	// Get viz manager
	ASLVizManager* GetVizManager() { return VizManager; };

	// Get viz sem map manager
	ASLVizSemMapManager* GetVizSemMapManager() { return VizSemMapManager; };

	// Get mongo manager
	ASLMongoQueryManager* GetMongoQueryManager() { return MongoQueryManager; };

	// Spawn or get manager from the world
	static ASLKnowrobManager* GetExistingOrSpawnNew(UWorld* World);

protected:
	// Setup user input bindings
	void SetupInputBindings();

	/* Knowrob websocket client delegate triggers */
	// KR retry connection timer callback
	void KRConnectRetryCallback();

	// Called when connected or disconnecetd with knowrob
	void OnKRConnection(bool bConnectionValue);

	// Called when a new message is received from knowrob
	void OnKRMsg();

private:
	// Get the mongo query manager from the world (or spawn a new one)
	bool SetMongoQueryManager();

	// Get the viz manager from the world (or spawn a new one)
	bool SetVizManager();

	// Get the viz semantic map manager from the world (or spawn a new one)
	bool SetVizSemMapManager();

	// Get the semantic map manager from the world (or spawn a new one)
    bool SetSemancticMapManager();

	// Get the control manager from the world (or spawn a new one)
    bool SetControlManager();

	// Get the symbolic logger from the world (or spawn a new one)
    bool SetSymbolicLogger();

	// Get the world state logger from the world (or spawn a new one)
	bool SetWorldStateLogger               ();

	/****************************************************************/
	/*							VizQ								*/
	/****************************************************************/
	// VizQ trigger
	void UserInputVizQActionTrigger();

	// Execute next query, return false if not more queries are available
	bool ExecuteNextQuery();

	// Execute the selected query (return false if index is not valid)
	bool ExecuteQuery(int32 Index);

protected:
	// Skip auto init and start
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bIgnore : 1;

	// True when all references are set and it is connected to the server
	uint8 bIsInit : 1;

	// True when active
	uint8 bIsStarted : 1;

	// True when done 
	uint8 bIsFinished : 1;

private:
	// Knowrob server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString KRServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 KRServerPort = 8080;
	
	// Websocket protocal
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString KRWSProtocol = TEXT("kr_websocket");

	// Retry connecting to knowrob
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bKRConnectRetry = false;

	// Interval at which to re-try to connect to the server
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bKRConnectRetry"))
	float KRConnectRetryInterval = 0.5f;

	// Max number of retrials (INDEX_NONE / -1 = infinite)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bKRConnectRetry"))
	int32 KRConnectRetryMaxNum = INDEX_NONE;


	// Mongo server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString MongoServerIP = TEXT("127.0.0.1");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 MongoServerPort = 27017;


	// Auto connect to mongodb at init
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLoadValuesFromCommandLine = true;

	// Websocket connection to knowrob
	TSharedPtr<FSLKRWSClient> KRWSClient;

	// KR reconnect timer handle
	FTimerHandle KRConnectRetryTimerHandle;

	// Number of reconnect retrials
	int32 KRConnectRetryNum;

	// Handle the protobuf message
	TSharedPtr<SLKRMsgDispatcher> KRMsgDispatcher;

	/* Managers */
	// Delegates mongo queries
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLMongoQueryManager* MongoQueryManager;

	// Marker visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizManager* VizManager;

	// Semantic map related visualization
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLVizSemMapManager* VizSemMapManager;

	// Manages loading semantic map
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    ASLLevelManager* LevelManager;

    // Manages controlling individual 
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    ASLControlManager* ControlManager;

	// Symbolic data logger
    UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
    ASLSymbolicLogger* SymbolicLogger;

	// Subsymbolic data logger
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLWorldStateLogger* WorldStateLogger;

	/****************************************************************/
	/*							VizQ								*/
	/****************************************************************/
	// Store predefined queries to execute
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	TArray<USLVizQBase*> Queries;

	// Auto init world to viz
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	bool bAutoConvertWorld = true;

	// User input trigger action name (Shift+V)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	FName UserInputActionName = "VizQTrigger";

	// Execute next query
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|VizQ")
	bool bTriggerButtonHack;

	// Current active query
	int32 QueryIndex = INDEX_NONE;

	/****************************************************************/
	/*					 Level Switch button hacks 			*/	
	/****************************************************************/
    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Map Buttons")
    FName LevelToSwitch = TEXT("");

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Map Buttons")
    bool bSwitchLevelButtonHack = false;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Map Buttons")
    bool bPrintAllMapsButtonHack = false;

	/****************************************************************/
    /*                        Control tests                         */
    /****************************************************************/
    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    FString IndividualToMove;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    FVector ControlLocation;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    FQuat ControlQuat;

	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
	float DelaySecond;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    bool bMoveIndividualButtonHack = false;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    TArray<FString> SelectedInividual;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    bool StartSelectedSimulationButtonHack = false;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Control Buttons")
    bool StopSelectedSimulationButtonHack = false;

	/****************************************************************/
    /*                        Symbolic Logger tests                 */
    /****************************************************************/
    // Symbolic Logger parameters
    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
    FSLSymbolicLoggerParams SymbolicLoggerParameters;

    // Location parameters
    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
    FSLLoggerLocationParams LocationParameters;
    
	// World state logger parameters
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
	FSLWorldStateLoggerParams WorldStateLoggerParameters;

	// DB server parameters
	UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
	FSLLoggerDBServerParams DBServerParameters;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
    bool StartLogButtonHack = false;

    UPROPERTY(EditAnywhere, Transient, Category = "Semantic Logger|Logger Buttons")
    bool StopLogButtonHack = false;
};
