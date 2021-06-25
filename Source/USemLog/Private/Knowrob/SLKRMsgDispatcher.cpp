// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen

#include "Knowrob/SLKRMsgDispatcher.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Knowrob/SLLevelManager.h"
#include "Control/SLControlManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizStructs.h"
#include "Runtime/SLSymbolicLogger.h"
#include "Runtime/SLLoggerStructs.h"
#include "Runtime/SLWorldStateLogger.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "TimerManager.h"

// Ctor
SLKRMsgDispatcher::SLKRMsgDispatcher()
{
}

// Dtor
SLKRMsgDispatcher::~SLKRMsgDispatcher()
{
}

// Set up required manager
void SLKRMsgDispatcher::Init(TSharedPtr<FSLKRWSClient> InKRWSClient, ASLMongoQueryManager* InMongoManger,
	ASLVizManager* InVizManager, ASLLevelManager* InLevelManager,
	ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger, 
	ASLWorldStateLogger* InWorldStateLogger, const FString InMongoSrvIP, int32 InMongoSrvPort)
{
	KRWSClient = InKRWSClient;
	MongoManager = InMongoManger;
	VizManager = InVizManager;
	LevelManager = InLevelManager;
	ControlManager = InControlManager;
	SymbolicLogger = InSymbolicLogger;
	WorldStateLogger = InWorldStateLogger;

	MongoServerIP = InMongoSrvIP;
	MongoServerPort = InMongoSrvPort;

	ControlManager->OnSimulationStart.BindRaw(this, &SLKRMsgDispatcher::SimulationStartResponse);
	ControlManager->OnSimulationFinish.BindRaw(this, &SLKRMsgDispatcher::SimulationStopResponse);
	bIsInit = true;
}

void SLKRMsgDispatcher::Reset()
{
	KRWSClient = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;
	LevelManager = nullptr;
	ControlManager = nullptr;
	SymbolicLogger = nullptr;
	ControlManager->OnSimulationFinish.Unbind();
	ControlManager->OnSimulationStart.Unbind();
	bIsInit = false;
}

// Parse the proto sequence and trigger function
void  SLKRMsgDispatcher::ProcessProtobuf(std::string ProtoStr)
{	
#if SL_WITH_PROTO
	sl_pb::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.functocall() == AmevaEvent.SetTask)
	{
		SetTask(AmevaEvent.settaskparam());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetEpisode)
	{
		SetEpisode(AmevaEvent.setepisodeparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerAt)
	{
		DrawMarker(AmevaEvent.drawmarkeratparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerTraj)
	{
		DrawMarkerTraj(AmevaEvent.drawmarkertrajparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.LoadLevel)
	{
		LoadLevel(AmevaEvent.loadlevelparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSimulation)
	{
		StartSimulation(AmevaEvent.startsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSimulation)
	{
		StopSimulation(AmevaEvent.stopsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartLogging)
	{
		StartLogging(AmevaEvent.startloggingparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopLogging)
	{
		StopLogging();
	}
	else if (AmevaEvent.functocall() == AmevaEvent.GetEpisodeData)
	{
		SendEpisodeData(AmevaEvent.getepisodedataparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetIndividualPose)
	{
		SetIndividualPose(AmevaEvent.setindividualposeparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.ApplyForceTo)
	{
		ApplyForceTo(AmevaEvent.applyforcetoparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.Highlight)
	{
		HighlightIndividual(AmevaEvent.highlightparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.RemoveHighlight)
	{
		RemoveIndividualHighlight(AmevaEvent.removehighlightparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.RemoveAllHighlight)
	{
		RemoveAllIndividualHighlight();
	}

#endif // SL_WITH_PROTO
}

#if SL_WITH_PROTO
// Set the task of MongoManager
void SLKRMsgDispatcher::SetTask(sl_pb::SetTaskParams params)
{
	const FString TaskId = FString(UTF8_TO_TCHAR(params.task().c_str()));
	bool bSuccess = MongoManager->SetTask(TaskId);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully set task id to %s .."), *TaskId, FPlatformTime::Seconds());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to set task id to %s .."), *TaskId, FPlatformTime::Seconds());
	}
	KRWSClient->SendResponse(Response);	
}

// Set the episode of MongoManager
void SLKRMsgDispatcher::SetEpisode(sl_pb::SetEpisodeParams params)
{
	const FString EpId = FString(UTF8_TO_TCHAR(params.episode().c_str()));
	bool bSuccess = MongoManager->SetEpisode(EpId);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully set episode id to %s .."), *EpId, FPlatformTime::Seconds());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to set episode id to %s .."), *EpId, FPlatformTime::Seconds());
	}
	KRWSClient->SendResponse(Response);
}

// Draw the individual marker
void SLKRMsgDispatcher::DrawMarker(sl_pb::DrawMarkerAtParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	float TimeStamp = params.timestamp();
	FVector Scale(params.scale());
	ESLVizPrimitiveMarkerType Type = GetMarkerType(params.marker());
	ESLVizMaterialType MaterialType = GetMarkerMaterialType(UTF8_TO_TCHAR(params.material().c_str()));
	FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(params.color().c_str()));
	FTransform Pose = MongoManager->GetIndividualPoseAt(Id, TimeStamp);
	TArray<FTransform> Poses;
	Poses.Add(Pose);
	VizManager->CreatePrimitiveMarker(Id, Poses, Type, params.scale(), Color, MaterialType);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Draw marker");
	KRWSClient->SendResponse(Response);
}

// Draw the individual trajectory
void SLKRMsgDispatcher::DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	float Start = params.start();
	float End = params.end();
	FVector Scale(params.scale());
	ESLVizPrimitiveMarkerType Type = GetMarkerType(params.marker());
	ESLVizMaterialType MaterialType = GetMarkerMaterialType(UTF8_TO_TCHAR(params.material().c_str()));
	FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(params.color().c_str()));
	TArray<FTransform> Poses = MongoManager->GetIndividualTrajectory(Id, Start, End);
	VizManager->CreatePrimitiveMarker(Id, Poses, Type, params.scale(), Color, MaterialType);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Draw trajectory");
	KRWSClient->SendResponse(Response);
}
// Hightlight the individual
void SLKRMsgDispatcher::HighlightIndividual(sl_pb::HighlightParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	ESLVizMaterialType MaterialType = GetMarkerMaterialType(UTF8_TO_TCHAR(params.material().c_str()));
	FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(params.color().c_str()));
	VizManager->HighlightIndividual(Id, Color, MaterialType);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Highlight individual");
	KRWSClient->SendResponse(Response);
}

// Remove the individual hightlight
void SLKRMsgDispatcher::RemoveIndividualHighlight(sl_pb::RemoveHighlightParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	VizManager->RemoveIndividualHighlight(Id);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Remove individual highlight");
	KRWSClient->SendResponse(Response);
}

// Hightlight the individual
void SLKRMsgDispatcher::RemoveAllIndividualHighlight()
{
	VizManager->RemoveAllIndividualHighlights();
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Remove individual highlight");
	KRWSClient->SendResponse(Response);
}

// Load the Semantic Map
void SLKRMsgDispatcher::LoadLevel(sl_pb::LoadLevelParams params)
{
	FString Level = UTF8_TO_TCHAR(params.level().c_str());
	LevelManager->SwitchLevelTo(FName(*Level));
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed - Switch level");
	KRWSClient->SendResponse(Response);
}

// Start Symbolic and World State Logger
void SLKRMsgDispatcher::StartLogging(sl_pb::StartLoggingParams params)
{
	FString TaskId = UTF8_TO_TCHAR(params.taskid().c_str());
	FString EpisodeId = UTF8_TO_TCHAR(params.episodeid().c_str());
	FSLSymbolicLoggerParams SymbolicLoggerParameters;
	FSLLoggerLocationParams LocationParameters;
	FSLWorldStateLoggerParams WorldStateLoggerParameters;
	FSLLoggerDBServerParams DBServerParameters;
	LocationParameters.bUseCustomTaskId = true;
	LocationParameters.TaskId = TaskId;
	LocationParameters.bUseCustomEpisodeId = true;
	LocationParameters.EpisodeId = EpisodeId;
	LocationParameters.bOverwrite = true;
	DBServerParameters.Ip = MongoServerIP;
	DBServerParameters.Port = MongoServerPort;
	
	SymbolicLogger->Init(SymbolicLoggerParameters, LocationParameters);
	WorldStateLogger->Init(WorldStateLoggerParameters, LocationParameters, DBServerParameters);
	
	bool bSuccess = true;
	if (!SymbolicLogger->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to init the symbolic logger.."),
			*FString(__FUNCTION__), __LINE__);
		bSuccess = false;
	}

	if (!WorldStateLogger->IsInit())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to init the world state logger.."),
			*FString(__FUNCTION__), __LINE__ );
		bSuccess = false;
	}

	if (bSuccess)
	{
		SymbolicLogger->Start();
		WorldStateLogger->Start();
	}

	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully started logging .."), FPlatformTime::Seconds());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to start loggers .."), FPlatformTime::Seconds());
	}
	KRWSClient->SendResponse(Response);
}

// Stop Symbolicand World State Logger
void SLKRMsgDispatcher::StopLogging()
{
	SymbolicLogger->Finish();
	WorldStateLogger->Finish();
	bool bSuccess = SymbolicLogger->IsFinished() && WorldStateLogger->IsFinished();

	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully finished logging .."), FPlatformTime::Seconds());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to finish loggers .."), FPlatformTime::Seconds());
	}
	KRWSClient->SendResponse(Response);
}

// Send the Symbolic log owl file
void SLKRMsgDispatcher::SendEpisodeData(sl_pb::GetEpisodeDataParams params)
{
	FString TaskId = UTF8_TO_TCHAR(params.taskid().c_str());
	FString EpisodeId = UTF8_TO_TCHAR(params.episodeid().c_str());
	const FString DirPath = FPaths::ProjectDir() + "/SL/Tasks/" + TaskId + "/";
	// Write experiment to file
	FString FullFilePath = DirPath + EpisodeId + TEXT("_ED.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	if (FPaths::FileExists(FullFilePath))
	{
		TArray<uint8> FileBinary;
		FSLKRResponse Response;
		Response.Type = ResponseType::FILE;
		Response.FileName = EpisodeId + TEXT("_ED.owl");
		FFileHelper::LoadFileToArray(Response.FileData, *FullFilePath);
		KRWSClient->SendResponse(Response);
	}
	else 
	{
		FSLKRResponse Response;
		Response.Type = ResponseType::TEXT;
		Response.Text = TEXT("Error: File not exists");
		KRWSClient->SendResponse(Response);
	}
}

// Start Simulation
void SLKRMsgDispatcher::StartSimulation(sl_pb::StartSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++) 
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}

	int32 Secs = params.duration();
	bool bSuccess = ControlManager->StartSimulationSelectionOnly(Ids, Secs);

	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully started simulation of %d individuals for %f secs.."),
			FPlatformTime::Seconds(), Ids.Num(), Secs);
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to start simulation of %d individuals for %f secs.."),
			FPlatformTime::Seconds(), Ids.Num(), Secs);
	}
	KRWSClient->SendResponse(Response);
}

// Stop Simulation
void SLKRMsgDispatcher::StopSimulation(sl_pb::StopSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++)
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}

	bool bSuccess = ControlManager->StopSimulationSelectionOnly(Ids);

	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully stopped simulation of %d individuals.."),
			FPlatformTime::Seconds(), Ids.Num());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to stop simulation of %d individuals.."),
			FPlatformTime::Seconds(), Ids.Num());
	}
	KRWSClient->SendResponse(Response);
}

// Move Individual
void SLKRMsgDispatcher::SetIndividualPose(sl_pb::SetIndividualPoseParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	FVector Loc = FVector(params.vecx(), params.vecy(), params.vecz());
	// TODO switch to X Y Z W, also in knowrob_ameva src/ue_control_cpp -> ue_set_individual_pose
	FQuat Quat = FQuat(params.quatw(), params.quatx(), params.quaty(), params.quatz());
	bool bSuccess = ControlManager->SetIndividualPose(Id, Loc, Quat);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully set pose of %s to [%s, %s].."),
			FPlatformTime::Seconds(), *Id, *Loc.ToString(), *Quat.ToString());
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to set pose of %s to [%s, %s].."),
			FPlatformTime::Seconds(), *Id, *Loc.ToString(), *Quat.ToString());
	}
	Response.Text = TEXT("Completed - Set individual pose");
	KRWSClient->SendResponse(Response);
}

void SLKRMsgDispatcher::ApplyForceTo(sl_pb::ApplyForceToParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	FVector Force = FVector(params.forcex(), params.forcey(), params.forcez());
	bool bSuccess = ControlManager->ApplyForceTo(Id, Force);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	if (bSuccess)
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Sucesfully applied force of [%s] to %s.."),
			FPlatformTime::Seconds(), *Force.ToString(), *Id);
	}
	else
	{
		Response.Text = FString::Printf(TEXT("[%.4f] Failed to apply force of [%s] to %s.."),
			FPlatformTime::Seconds(), *Force.ToString(), *Id);
	}
	KRWSClient->SendResponse(Response);
}

// Transform the maker type
ESLVizPrimitiveMarkerType SLKRMsgDispatcher::GetMarkerType(sl_pb::MarkerType Marker)
{
	if (Marker == sl_pb::Sphere)
	{
		return ESLVizPrimitiveMarkerType::Sphere;
	}
	else if (Marker == sl_pb::Cylinder)
	{
		return ESLVizPrimitiveMarkerType::Cylinder;
	}
	else if (Marker == sl_pb::Arrow)
	{
		return ESLVizPrimitiveMarkerType::Arrow;
	}
	else if (Marker == sl_pb::Axis)
	{
		return ESLVizPrimitiveMarkerType::Axis;
	}
	else if (Marker == sl_pb::Box) 
	{
		return ESLVizPrimitiveMarkerType::Box;
	}
	return ESLVizPrimitiveMarkerType::NONE;
}
#endif // SL_WITH_PROTO

// Transform the string to color
FLinearColor SLKRMsgDispatcher::GetMarkerColor(const FString &Color)
{
	if (Color.Equals("Red", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Red;
	}
	else if (Color.Equals("Black", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Black;
	}
	else if (Color.Equals("Blue", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Blue;
	}
	else if (Color.Equals("Gray", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Gray;
	}
	else if (Color.Equals("Yellow", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Yellow;
	}
	else if (Color.Equals("Green", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Green;
	}
	return FLinearColor::White;
}

// Transform the material type
ESLVizMaterialType SLKRMsgDispatcher::GetMarkerMaterialType(const FString& MaterialType)
{
	if (MaterialType.Equals("Lit", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Lit;
	}
	else if (MaterialType.Equals("Unlit", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Unlit;
	}
	else if (MaterialType.Equals("Additive", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Additive;
	}
	else if (MaterialType.Equals("Translucent", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Translucent;
	}
	return ESLVizMaterialType::NONE;
}

// Send response when simulation stop
void SLKRMsgDispatcher::SimulationStopResponse()
{
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = FString::Printf(TEXT("[%.4f] Simulation finished.."),FPlatformTime::Seconds());
	KRWSClient->SendResponse(Response);
}

void SLKRMsgDispatcher::SimulationStartResponse()
{
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = FString::Printf(TEXT("[%.4f] Simulation started.."), FPlatformTime::Seconds());
	KRWSClient->SendResponse(Response);
}