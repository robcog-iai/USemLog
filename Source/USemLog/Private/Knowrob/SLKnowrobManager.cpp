// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobManager.h"
#include "Control/SLControlManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizSemMapManager.h"
#include "VizQ/SLVizQBase.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Runtime/SLSymbolicLogger.h"
#include "Runtime/SLWorldStateLogger.h"
#include "TimerManager.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLKnowrobManager::ASLKnowrobManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIgnore = false;
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.5;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLKnowrob"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Dtor
ASLKnowrobManager::~ASLKnowrobManager()
{
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLKnowrobManager::BeginPlay()
{
	Super::BeginPlay();	

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's ignore flag is true, skipping"), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Load values from the commandline
	if (bLoadValuesFromCommandLine)
	{
		FParse::Value(FCommandLine::Get(), TEXT("KRServerIP="), KRServerIP);
		FParse::Value(FCommandLine::Get(), TEXT("KRServerPort="), KRServerPort);
		FParse::Value(FCommandLine::Get(), TEXT("KRProtocol="), KRWSProtocol);
		FParse::Value(FCommandLine::Get(), TEXT("MongoServerIP="), MongoServerIP);
		FParse::Value(FCommandLine::Get(), TEXT("MongoServerPort="), MongoServerPort);
	}

	Init();
	Start();

}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLKnowrobManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;
		
	/* VizQ */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bTriggerButtonHack))
	{
		bTriggerButtonHack = false;
		if (!VizManager || !VizManager->IsValidLowLevel() || VizManager->IsPendingKillOrUnreachable()
			|| !VizManager->IsInit() || !VizManager->IsWorldConvertedToVisualizationMode())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Viz manager is not valid, init or world is not converted.. "), *FString(__FUNCTION__), __LINE__);
			return;
		}
		if (!MongoQueryManager || !MongoQueryManager->IsValidLowLevel() || MongoQueryManager->IsPendingKillOrUnreachable()
			|| !MongoQueryManager->IsConnected())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d MongoQueryManager is not valid or not connected.. "), *FString(__FUNCTION__), __LINE__);
			return;
		}
		if(ExecuteNextQuery())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Executed query %d.. "),
				*FString(__FUNCTION__), __LINE__, QueryIndex);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No more queries to execute (last=%d).. "),
				*FString(__FUNCTION__), __LINE__, QueryIndex);
		}
	}

    /* Map button hacks */
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bSwitchLevelButtonHack))
    {
        bSwitchLevelButtonHack = false;
        LevelManager->SwitchLevelTo(LevelToSwitch);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bPrintAllMapsButtonHack))
    {
        bPrintAllMapsButtonHack = false;
        LevelManager->GetAllLevel();
    }

    /*    Control Individual    */
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bMoveIndividualButtonHack))
    {
        bMoveIndividualButtonHack = false;
        ControlManager->SetIndividualPose(IndividualToMove, ControlLocation, ControlQuat);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, StartSelectedSimulationButtonHack))
    {
        StartSelectedSimulationButtonHack = false;
        ControlManager->StartSimulationSelectionOnly(SelectedInividual, DelaySecond);
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, StopSelectedSimulationButtonHack))
    {
        StopSelectedSimulationButtonHack = false;
        ControlManager->StopSimulationSelectionOnly(SelectedInividual);
    }
	
    /*    Symbolic logging    */
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, StartLogButtonHack))
    {
        StartLogButtonHack = false;
		SymbolicLogger->Init(SymbolicLoggerParameters, LocationParameters);
		if (!SymbolicLogger->IsInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init the symbolic logger.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}
	
		WorldStateLogger->Init(WorldStateLoggerParameters, LocationParameters, DBServerParameters);
		if (!WorldStateLogger->IsInit())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init the world state logger.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			return;
		}

        SymbolicLogger->Start();
		WorldStateLogger->Start();
    }
    else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, StopLogButtonHack))
    {
        StopLogButtonHack = false;
        SymbolicLogger->Finish();
		WorldStateLogger->Finish();
    }
}
#endif // WITH_EDITOR

// Called every frame
void ASLKnowrobManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when actor removed from game or game ended
void ASLKnowrobManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

// Set up any required references and connect to server
void ASLKnowrobManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already initialized.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Initialize KR connection client
	if (!KRWSClient.IsValid())
	{
		KRWSClient = MakeShareable<FSLKRWSClient>(new FSLKRWSClient());
		KRWSClient->Init(KRServerIP, KRServerPort, KRWSProtocol);
	}

	// Get and connect the mongo query manager
	if (!SetMongoQueryManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not get access to the mongo query manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!MongoQueryManager->Connect(MongoServerIP, MongoServerPort))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not connect to mongo db.. (%s::%d)"),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MongoServerIP, MongoServerPort);
		return;
	}

	// Get an initialise the visualization manager
	if (!SetVizManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not get access to the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	VizManager->Init();
	if (!VizManager->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (bAutoConvertWorld)
	{
		VizManager->ConvertWorldToVisualizationMode();
	}

	// Get the map manager
    if (!SetSemancticMapManager())
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not get access to the map manager.."),
            *FString(__FUNCTION__), __LINE__, *GetName());
        return;
    }
	LevelManager->Init();
	if (!LevelManager->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not init the map manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
       
	// Get the control manager
    if (!SetControlManager())
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not get access to the control manager.."),
            *FString(__FUNCTION__), __LINE__, *GetName());
        return;
    }
	ControlManager->Init();
    if (!ControlManager->IsInit())
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not init the control manager.."),
            *FString(__FUNCTION__), __LINE__, *GetName());
        return;
    }

    // Get the symbolic logger
    if (!SetSymbolicLogger())
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager(%s) could not set the symbolic logger.."),
            *FString(__FUNCTION__), __LINE__, *GetName());
    }

	// Get the world state logger
	if (!SetWorldStateLogger())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not set the world state logger.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}

	// Get and init the sem map visualizer
	if (!SetVizSemMapManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not get access to the viz sem map manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	VizSemMapManager->Init();
	if (!VizSemMapManager->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init the viz sem map manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLKnowrobManager::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already started.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized, cannot start.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	
	if (KRWSClient.IsValid())
	{
		// Bind delegates of the knowrob websocket client
		KRWSClient->OnConnection.BindUObject(this, &ASLKnowrobManager::OnKRConnection);
		KRWSClient->OnNewProcessedMsg.BindUObject(this, &ASLKnowrobManager::OnKRMsg);

		// Try connection
		KRWSClient->Connect();
	}

	// Initialize dispatcher for parsing protobuf message
	KRMsgDispatcher = MakeShareable<SLKRMsgDispatcher>(new SLKRMsgDispatcher());
	KRMsgDispatcher->Init(KRWSClient, MongoQueryManager, VizManager, LevelManager, ControlManager, SymbolicLogger, WorldStateLogger, MongoServerIP, MongoServerPort);
	if (!KRMsgDispatcher->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not init kr msg dispatcher.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Bind user inputs
	SetupInputBindings();

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLKnowrobManager::Finish(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit && !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not initialized nor started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (KRWSClient.IsValid())
	{
		KRWSClient->Disconnect();
		KRWSClient->Clear();
		KRWSClient->OnNewProcessedMsg.Unbind();
		KRWSClient->OnConnection.Unbind();
		KRWSClient.Reset();
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d %s succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Spawn or get manager from the world
ASLKnowrobManager* ASLKnowrobManager::GetExistingOrSpawnNew(UWorld* World)
{
	// Check in world
	for (TActorIterator<ASLKnowrobManager>Iter(World); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			return *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_KnowrobManager");
	auto Manager = World->SpawnActor<ASLKnowrobManager>(SpawnParams);
#if WITH_EDITOR
	Manager->SetActorLabel(TEXT("SL_KnowrobManager"));
#endif // WITH_EDITOR
	return Manager;
}

// Setup user input bindings
void ASLKnowrobManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(UserInputActionName, IE_Pressed, this, &ASLKnowrobManager::UserInputVizQActionTrigger);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access player controller (make sure it is not called before BeginPlay).."), *FString(__FUNCTION__), __LINE__);
	}
}

// KR retry connection timer callback
void ASLKnowrobManager::KRConnectRetryCallback()
{
	if (KRWSClient.IsValid())
	{
		if (!KRWSClient->IsConnected())
		{
			if (KRConnectRetryMaxNum == INDEX_NONE)
			{
				KRConnectRetryNum++;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d KRWSClient connect retry num %d.."),
					*FString(__FUNCTION__), __LINE__, KRConnectRetryNum);
				KRWSClient->Connect();
			}
			else if(KRConnectRetryNum <= KRConnectRetryMaxNum)
			{
				KRConnectRetryNum++;
				UE_LOG(LogTemp, Warning, TEXT("%s::%d KRWSClient connect retry num %d/%d.."),
					*FString(__FUNCTION__), __LINE__, KRConnectRetryNum, KRConnectRetryMaxNum);
				KRWSClient->Connect();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d KRWSClient is already connected.."), *FString(__FUNCTION__), __LINE__);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d KRWSClient is not valid, cannot reconnect.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Called when connected or disconnecetd with knowrob
void ASLKnowrobManager::OnKRConnection(bool bConnectionValue)
{
	if (bConnectionValue)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s connected to knowrob.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
	else if(bKRConnectRetry)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s is disconnected from knowrob, retrying in %f seconds.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), KRConnectRetryInterval);

		GetWorld()->GetTimerManager().SetTimer(KRConnectRetryTimerHandle,
			this, &ASLKnowrobManager::KRConnectRetryCallback, KRConnectRetryInterval);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d %s knowrob connection is closed.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
}

// Called when a new message is received from knowrob
void ASLKnowrobManager::OnKRMsg()
{
	std::string ProtoMsgBinary;
	while (KRWSClient->MessageQueue.Dequeue(ProtoMsgBinary))
	{
#if SL_WITH_PROTO
		UE_LOG(LogTemp, Log, TEXT("%s::%d Processing message.."), *FString(__FUNCTION__), __LINE__);
		KRMsgDispatcher->ProcessProtobuf(ProtoMsgBinary);
#endif // SL_WITH_PROTO	
	}
}

// Get the mongo query manager from the world (or spawn a new one)
bool ASLKnowrobManager::SetMongoQueryManager()
{
	if (MongoQueryManager && MongoQueryManager->IsValidLowLevel() && !MongoQueryManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLMongoQueryManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			MongoQueryManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_MongoQueryManager");
	MongoQueryManager = GetWorld()->SpawnActor<ASLMongoQueryManager>(SpawnParams);
#if WITH_EDITOR
	MongoQueryManager->SetActorLabel(TEXT("SL_MongoQueryManager"));
#endif // WITH_EDITOR
	return true;
}

// Get the viz manager from the world (or spawn a new one)
bool ASLKnowrobManager::SetVizManager()
{
	if (VizManager && VizManager->IsValidLowLevel() && !VizManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizManager");
	VizManager = GetWorld()->SpawnActor<ASLVizManager>(SpawnParams);
#if WITH_EDITOR
	VizManager->SetActorLabel(TEXT("SL_VizManager"));
#endif // WITH_EDITOR
	return true;
}

// Get the viz semantic map manager from the world (or spawn a new one)
bool ASLKnowrobManager::SetVizSemMapManager()
{
	if (VizSemMapManager && VizSemMapManager->IsValidLowLevel() && !VizSemMapManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLVizSemMapManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			VizSemMapManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_VizSemMapManager");
	VizSemMapManager = GetWorld()->SpawnActor<ASLVizSemMapManager>(SpawnParams);
#if WITH_EDITOR
	VizSemMapManager->SetActorLabel(TEXT("SL_VizSemMapManager"));
#endif // WITH_EDITOR
	return true;
}


// Get the semantic map manager from the world (or spawn a new one)
bool ASLKnowrobManager::SetSemancticMapManager()
{
    if (LevelManager && LevelManager->IsValidLowLevel() && !LevelManager->IsPendingKillOrUnreachable())
    {
        return true;
    }

    for (TActorIterator<ASLLevelManager>Iter(GetWorld()); Iter; ++Iter)
    {
        if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
        {
            LevelManager = *Iter;
            return true;
        }
    }

    // Spawning a new manager
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("SL_MapManager");
    LevelManager = GetWorld()->SpawnActor<ASLLevelManager>(SpawnParams);
#if WITH_EDITOR
    LevelManager->SetActorLabel(TEXT("SL_MapManager"));
#endif // WITH_EDITOR
    return true;
}

// Get the control manager from the world (or spawn a new one)
bool ASLKnowrobManager::SetControlManager()
{
    if (ControlManager && ControlManager->IsValidLowLevel() && !ControlManager->IsPendingKillOrUnreachable())
    {
        return true;
    }

    for (TActorIterator<ASLControlManager>Iter(GetWorld()); Iter; ++Iter)
    {
        if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
        {
            ControlManager = *Iter;
            return true;
        }
    }

    // Spawning a new manager
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("SL_ControlManager");
    ControlManager = GetWorld()->SpawnActor<ASLControlManager>(SpawnParams);
#if WITH_EDITOR
    ControlManager->SetActorLabel(TEXT("SL_ControlManager"));
#endif // WITH_EDITOR
    return true;
}

// Get the symbolic logger from the world (or spawn a new one)
bool ASLKnowrobManager::SetSymbolicLogger()
{
    if (SymbolicLogger && SymbolicLogger->IsValidLowLevel() && !SymbolicLogger->IsPendingKillOrUnreachable())
    {
        return true;
    }

    for (TActorIterator<ASLSymbolicLogger>Iter(GetWorld()); Iter; ++Iter)
    {
        if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
        {
            SymbolicLogger = *Iter;
            return true;
        }
    }

    // Spawning a new manager
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("SL_SymbolLogger");
    SymbolicLogger = GetWorld()->SpawnActor<ASLSymbolicLogger>(SpawnParams);
#if WITH_EDITOR
    SymbolicLogger->SetActorLabel(TEXT("SL_SymbolLogger"));
#endif // WITH_EDITOR
    return true;
}

// Get the symbolic logger from the world (or spawn a new one)
bool ASLKnowrobManager::SetWorldStateLogger()
{
	if (WorldStateLogger && WorldStateLogger->IsValidLowLevel() && !WorldStateLogger->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLWorldStateLogger>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			WorldStateLogger = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_WorldStateLogger");
	SymbolicLogger = GetWorld()->SpawnActor<ASLSymbolicLogger>(SpawnParams);
#if WITH_EDITOR
	SymbolicLogger->SetActorLabel(TEXT("SL_WorldStateLogger"));
#endif // WITH_EDITOR
	return true;
}

/****************************************************************/
/*							VizQ								*/
/****************************************************************/
// VizQ trigger
void ASLKnowrobManager::UserInputVizQActionTrigger()
{
	if (ExecuteNextQuery())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Executed query %d.. "),
			*FString(__FUNCTION__), __LINE__, QueryIndex);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No more queries to execute (last=%d).. "),
			*FString(__FUNCTION__), __LINE__, QueryIndex);
	}
}

// Execute next query, return false if not more queries are available
bool ASLKnowrobManager::ExecuteNextQuery()
{
	if (ExecuteQuery(QueryIndex + 1))
	{
		QueryIndex++;
		return true;
	}
	return false;
}

// Execute the selected query (return false if index is not valid)
bool ASLKnowrobManager::ExecuteQuery(int32 Index)
{
	if (Queries.IsValidIndex(Index))
	{
		USLVizQBase* QueryObj = Queries[Index];
		if (QueryObj && QueryObj->IsValidLowLevel())
		{
			QueryObj->Execute(this);
			return true;
		}
	}
	return false;
}