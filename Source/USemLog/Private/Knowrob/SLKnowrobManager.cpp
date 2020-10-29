// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "TimerManager.h"

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
	if (!IsTemplate() && !bIsFinished && (bIsStarted || bIsInit))
	{
		Finish(true);
	}
}

// Called when the game starts or when spawned
void ASLKnowrobManager::BeginPlay()
{
	Super::BeginPlay();	

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

	/* Episode data hacks */
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

	/* VIZ episode replay */
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bSetupWorldForEpisodeReplayButtonHack))
	{
		bSetupWorldForEpisodeReplayButtonHack = false;
		if (!bIsInit) { return; }
		VizManager->ConvertWorldToVisualizationMode();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bLoadEpisodeDataButtonHack))
	{
		bLoadEpisodeDataButtonHack = false;
		if (!bIsInit) { return; }
		if (!VizManager->IsWorldConvertedToVisualizationMode())
		{
			VizManager->ConvertWorldToVisualizationMode();
		}
		VizManager->LoadEpisodeData(MongoQueryManager->GetEpisodeData(TaskIdValueHack, EpisodeIdValueHack));
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bGotoButtonHack))
	{
		bGotoButtonHack = false;
		if (!bIsInit) { return; }
		if (!VizManager->IsWorldConvertedToVisualizationMode())
		{
			VizManager->ConvertWorldToVisualizationMode();
		}
		if (!VizManager->IsEpisodeLoaded())
		{
			VizManager->LoadEpisodeData(MongoQueryManager->GetEpisodeData(TaskIdValueHack, EpisodeIdValueHack));
		}
		VizManager->GotoEpisodeFrame(GotoValueHack);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bReplayButtonHack))
	{
		bReplayButtonHack = false;
		if (!bIsInit) { return; }
		if (!VizManager->IsWorldConvertedToVisualizationMode())
		{
			VizManager->ConvertWorldToVisualizationMode();
		}
		if (!VizManager->IsEpisodeLoaded())
		{
			VizManager->LoadEpisodeData(MongoQueryManager->GetEpisodeData(TaskIdValueHack, EpisodeIdValueHack));
		}
		if (ReplayBeginValueHack > 0 && ReplayEndValueHack > 0 && ReplayBeginValueHack < ReplayEndValueHack)
		{
			FSLVizEpisodePlayParams PlayParams;
			PlayParams.bLoop = bReplayLoopValueHack;
			PlayParams.UpdateRate = ReplayUpdateRateValueHack;
			VizManager->PlayEpisodeTimeline(ReplayBeginValueHack, ReplayEndValueHack, PlayParams);
		}
		else
		{
			FSLVizEpisodePlayParams PlayParams;
			PlayParams.bLoop = bReplayLoopValueHack;
			PlayParams.UpdateRate = ReplayUpdateRateValueHack;
			VizManager->PlayEpisode(PlayParams);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bReplayPauseButtonHack))
	{
		if (!bIsInit) { return; }
		VizManager->PauseReplay(bReplayPauseButtonHack);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bReplayStopButtonHack))
	{
		bReplayStopButtonHack = false;
		if (!bIsInit) { return; }
		VizManager->StopReplay();
	}

	/*	VIZ highlights	*/
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bDrawSelectedHighlights))
	{
		bDrawSelectedHighlights = false;
		if (!bIsInit) { return; }
		for (const auto HC : HighlightValuesHack)
		{
			VizManager->HighlightIndividual(HC.IndividualId, HC.Color, HC.MaterialType);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bUpdateHighlights))
	{
		bUpdateHighlights = false;
		if (!bIsInit) { return; }
		for (const auto HC : HighlightValuesHack)
		{
			VizManager->UpdateIndividualHighlight(HC.IndividualId, HC.Color, HC.MaterialType);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bRemoveAllHighlights))
	{
		bRemoveAllHighlights = false;
		if (!bIsInit) { return; }
		VizManager->RemoveAllIndividualHighlights();
	}

	/*	VIZ markers	*/
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bDrawMarkers))
	{
		bDrawMarkers = false;
		if (!bIsInit) { return; }
		for (const auto M : MarkerValuesHack)
		{
			TArray<FTransform> Poses;
			TArray<TPair<FTransform, TMap<int32, FTransform>>> SkeletalPoses;

			if (M.EndTime > 0)
			{
				if (!M.bIsSkeletal)
				{
					Poses = MongoQueryManager->GetIndividualTrajectory(TaskIdValueHack, EpisodeIdValueHack,
						M.IndividualId, M.StartTime, M.EndTime, M.DeltaT);
				}
				else
				{
					SkeletalPoses = MongoQueryManager->GetSkeletalIndividualTrajectory(TaskIdValueHack, EpisodeIdValueHack,
						M.IndividualId, M.StartTime, M.EndTime, M.DeltaT);
				}
			}
			else
			{
				if (!M.bIsSkeletal)
				{
					Poses.Add(MongoQueryManager->GetIndividualPoseAt(TaskIdValueHack, EpisodeIdValueHack,
						M.IndividualId, M.StartTime));
				}
				else
				{
					SkeletalPoses.Add(MongoQueryManager->GetSkeletalIndividualPoseAt(TaskIdValueHack, EpisodeIdValueHack,
						M.IndividualId, M.StartTime));
				}
			}
			
			if (Poses.Num() == 0 && SkeletalPoses.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Empty poses array.."), *FString(__FUNCTION__), __LINE__);
				return;
			}

			if (M.bAsTimeline)
			{
				if (Poses.Num() < 2 && SkeletalPoses.Num() < 2)
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Not enought poses for a timeline.."), *FString(__FUNCTION__), __LINE__);
					return;
				}

				if (M.bIsSkeletal)
				{
					if (M.bUseCustomColor)
					{
						VizManager->CreateSkeletalMeshMarkerTimeline(M.MarkerId, SkeletalPoses,
							M.IndividualId, M.Color, M.MaterialType,
							M.Duration, M.bLoop, M.UpdateRate);
					}
					else
					{
						VizManager->CreateSkeletalMeshMarkerTimeline(M.MarkerId, SkeletalPoses, M.IndividualId,
							M.Duration, M.bLoop, M.UpdateRate);
					}
				}
				else if (M.bUsePrimitiveMesh)
				{
					VizManager->CreatePrimitiveMarkerTimeline(M.MarkerId, Poses,
						M.PrimitiveType, M.Size, M.Color, M.MaterialType, 
						M.Duration, M.bLoop, M.UpdateRate);
				}
				else
				{
					if (M.bUseCustomColor)
					{
						VizManager->CreateStaticMeshMarkerTimeline(M.MarkerId, Poses, 
							M.IndividualId, M.Color, M.MaterialType,
							M.Duration, M.bLoop, M.UpdateRate);
					}
					else
					{
						VizManager->CreateStaticMeshMarkerTimeline(M.MarkerId, Poses, M.IndividualId,
							M.Duration, M.bLoop, M.UpdateRate);
					}
				}
			}
			else
			{
				if (M.bIsSkeletal)
				{
					if (M.bUseCustomColor)
					{
						VizManager->CreateSkeletalMeshMarker(M.MarkerId, SkeletalPoses,
							M.IndividualId, M.Color, M.MaterialType);
					}
					else
					{
						VizManager->CreateSkeletalMeshMarker(M.MarkerId, SkeletalPoses, M.IndividualId);
					}
				}
				else if (M.bUsePrimitiveMesh)
				{
					VizManager->CreatePrimitiveMarker(M.MarkerId, Poses, M.PrimitiveType, M.Size, M.Color, M.MaterialType);
				}
				else
				{
					if (M.bUseCustomColor)
					{
						VizManager->CreateStaticMeshMarker(M.MarkerId, Poses, M.IndividualId, M.Color, M.MaterialType);
					}
					else
					{
						VizManager->CreateStaticMeshMarker(M.MarkerId, Poses, M.IndividualId);
					}
				}
			}
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bRemoveAllMarkers))
	{
		bRemoveAllMarkers = false;
		if (!bIsInit) { return; }
		VizManager->RemoveAllMarkers();
	}

	/* MONGO query button hacks */
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
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) is already initialized.."),
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

	// Get an initialise the visualization manager
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
	if (bAutoConvertWorld)
	{
		VizManager->ConvertWorldToVisualizationMode();
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
	
	if (KRWSClient.IsValid())
	{
		// Bind delegates of the knowrob websocket client
		KRWSClient->OnConnection.BindUObject(this, &ASLKnowrobManager::OnKRConnection);
		KRWSClient->OnNewProcessedMsg.BindUObject(this, &ASLKnowrobManager::OnKRMsg);

		// Try connection
		KRWSClient->Connect();
	}

	// Initialize dispatcher for parsing protobuf message
	KREventDispatcher = MakeShareable<FSLKREventDispatcher>(new FSLKREventDispatcher(MongoQueryManager, VizManager));

	// Bind user inputs
	SetupInputBindings();

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
		KRWSClient->Clear();
		KRWSClient->OnNewProcessedMsg.Unbind();
		KRWSClient->OnConnection.Unbind();
		KRWSClient.Reset();
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d Knowrob manager (%s) succesfully finished.."),
		*FString(__FUNCTION__), __LINE__, *GetName());
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
		UE_LOG(LogTemp, Log, TEXT("%s::%d Knowrob manager (%s) connected to knowrob.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
	else if(bKRConnectRetry)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Knowrob manager (%s) is disconnected from knowrob, retrying in %f seconds.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), KRConnectRetryInterval);

		GetWorld()->GetTimerManager().SetTimer(KRConnectRetryTimerHandle,
			this, &ASLKnowrobManager::KRConnectRetryCallback, KRConnectRetryInterval);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Knowrob manager (%s) knowrob connection is closed.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
	}
}

// Called when a new message is received from knowrob
void ASLKnowrobManager::OnKRMsg()
{
	std::string ProtoMsgBinary;
	while (KRWSClient->MessageQueue.Dequeue(ProtoMsgBinary))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Processing message.."), *FString(__FUNCTION__), __LINE__);
		KREventDispatcher->ProcessProtobuf(ProtoMsgBinary);
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
		if (Queries[Index]->IsValidLowLevel())
		{
			Queries[Index]->Execute(VizManager, MongoQueryManager);
			return true;
		}
	}
	return false;
}