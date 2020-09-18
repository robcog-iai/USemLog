// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLKnowrobManager::ASLKnowrobManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
	//if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	//{
	//	Finish(true);
	//}
}

// Called when the game starts or when spawned
void ASLKnowrobManager::BeginPlay()
{
	Super::BeginPlay();	
	//Init();
	//Start();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLKnowrobManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bInitButtonHack))
	{
		bInitButtonHack = false;
		Init();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bStartButtonHack))
	{
		bStartButtonHack = false;
		Start();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bFinishButtonHack))
	{
		bFinishButtonHack = false;
		Finish();
	}

	/* VIZ button hacks */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bSetupWorldForEpisodeReplayButtonHack))
	{
		bSetupWorldForEpisodeReplayButtonHack = false;
		if (!bIsInit) { return; }
		VizManager->SetupWorldForEpisodeReplay();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bLoadEpisodeDataButtonHack))
	{
		bLoadEpisodeDataButtonHack = false;
		if (!bIsInit) { return; }
		VizManager->LoadEpisodeData(MongoQueryManager->GetEpisodeData(TaskIdValueHack, EpisodeIdValueHack));
	}

	/* MONGO button hacks */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bMongoConnectButtonHack))
	{
		bMongoConnectButtonHack = false;
		if (!bIsInit) { return; }
		if (MongoQueryManager->Connect(MongoServerIP, MongoServerPort))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Connected to %s:%ld"), *FString(__FUNCTION__), __LINE__, *MongoServerIP, MongoServerPort);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to connect to %s:%ld"), *FString(__FUNCTION__), __LINE__, *MongoServerIP, MongoServerPort);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bMongoDisconnectButtonHack))
	{
		bMongoDisconnectButtonHack = false;
		if (!bIsInit) { return; }
		MongoQueryManager->Disconnect();
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Disconnected.."), *FString(__FUNCTION__), __LINE__);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bSetTaskButtonhack))
	{
		bSetTaskButtonhack = false;
		if (!bIsInit) { return; }
		if (MongoQueryManager->SetTask(TaskIdValueHack))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Set task to %s"), *FString(__FUNCTION__), __LINE__, *TaskIdValueHack);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to set task to %s"), *FString(__FUNCTION__), __LINE__, *TaskIdValueHack);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bSetEpisodeButtonHack))
	{
		bSetEpisodeButtonHack = false;
		if (!bIsInit) { return; }
		if (MongoQueryManager->SetEpisode(EpisodeIdValueHack))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Set episode to %s"), *FString(__FUNCTION__), __LINE__, *EpisodeIdValueHack);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to set episode to %s"), *FString(__FUNCTION__), __LINE__, *EpisodeIdValueHack);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bPoseQueryButtonHack))
	{
		bPoseQueryButtonHack = false;
		if (!bIsInit) { return; }

		FTransform Pose = MongoQueryManager->GetIndividualPoseAt(TaskIdValueHack, EpisodeIdValueHack,
			IndividualIdValueHack, StartTimestampValueHack);

		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f] IndividualPose: Loc=%s; \t Quat=%s; \t (%s:%s:%s)"),
			*FString(__FUNCTION__), __LINE__, StartTimestampValueHack, *Pose.GetLocation().ToString(), *Pose.GetRotation().ToString(),
			*TaskIdValueHack, *EpisodeIdValueHack, *IndividualIdValueHack);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bTrajectoryQueryButtonHack))
	{
		bTrajectoryQueryButtonHack = false;
		if (!bIsInit) { return; }

		TArray<FTransform> Trajectory = MongoQueryManager->GetIndividualTrajectory(TaskIdValueHack, EpisodeIdValueHack,
			IndividualIdValueHack, StartTimestampValueHack, EndTimestampValueHack, DeltaTValueHack);

		UE_LOG(LogTemp, Warning, TEXT("%s::%d [%f-%f] IndividualTrajectoryNum=%ld; (%s:%s:%s)"),
			*FString(__FUNCTION__), __LINE__, StartTimestampValueHack, EndTimestampValueHack, Trajectory.Num(),
			*TaskIdValueHack, *EpisodeIdValueHack, *IndividualIdValueHack);

		for (const auto& Pose : Trajectory)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t\t\t\t Loc=%s; \t Quat=%s;"), *Pose.GetLocation().ToString(), *Pose.GetRotation().ToString());
		}
	}
	
}
#endif // WITH_EDITOR

// Called every frame
void ASLKnowrobManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Set up any required references and connect to server
void ASLKnowrobManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is already initialized.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Initialize connection with knowrob
	if (!KRWSClient.IsValid())
	{
		KRWSClient = MakeShareable<FSLKRWSClient>(new FSLKRWSClient());
	}
	KRWSClient->Connect(KRServerIP, KRServerPort, KRWSProtocol);

	if (!SetVizManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not get access to the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!VizManager->Init())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not init the viz manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Get and connect the mongo query manager
	if (!SetMongoQueryManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not get access to the mongo query manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}
	if (!MongoQueryManager->Connect(MongoServerIP, MongoServerPort))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Knowrob manager (%s) could not connect to mongo db.. (%s::%d)"),
			*FString(__FUNCTION__), __LINE__, *GetName(), *MongoServerIP, MongoServerPort);
		return;
	}

	bIsInit = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) succesfully initialized.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Start processing any incomming messages
void ASLKnowrobManager::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is already started.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is not initialized, cannot start.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Bind delegates of the knowrob websocket client
	KRWSClient->OnNewProcessedMsg.BindUObject(this, &ASLKnowrobManager::OnKRMsg);
	KRWSClient->OnConnection.BindUObject(this, &ASLKnowrobManager::OnKRConnection);

	bIsStarted = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) succesfully started.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Stop processing the messages, and disconnect from server
void ASLKnowrobManager::Finish(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is already finished.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit && !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is not initialized nor started, cannot finish.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (KRWSClient.IsValid())
	{
		KRWSClient->Disconnect();
		KRWSClient->OnNewProcessedMsg.Unbind();
		KRWSClient->OnConnection.Unbind();
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
}

// Called when a new message is received from knowrob
void ASLKnowrobManager::OnKRMsg()
{
	std::string ProtoMsgBinary;
	while (KRWSClient->MessageQueue.Dequeue(ProtoMsgBinary))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Processing message.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Called when connected or disconnecetd with knowrob
void ASLKnowrobManager::OnKRConnection(bool bConnectionValue)
{
	if (bConnectionValue)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Knowrob manager (%s) succesfully connected to knowrob.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Knowrob manager (%s) disconnected from knowrob.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
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
